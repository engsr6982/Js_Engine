#include "JavaScriptPluginLoader.h"
#include "CppObjectMapper.h"
#include "DataTransfer.h"
#include "Entry.h"
#include "JavaScriptPlugin.h"
#include "NodeHelper.h"
#include "ScriptBackend.hpp"
#include "endstone/detail/server.h"
#include "fmt/format.h"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "node.h"
#include "uv.h"
#include "v8-context.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-locker.h"
#include "v8-value.h"
#include <filesystem>
#include <stop_token>
#include <string>
#include <thread>


class TestClass {
public:
    TestClass(int p) {
        std::cout << "TestClass(" << p << ")" << std::endl;
        X = p;
    }

    static void Print(std::string msg) { std::cout << msg << std::endl; }

    int Add(int a, int b) {
        std::cout << "Add(" << a << "," << b << ")" << std::endl;
        return a + b;
    }

    int X;
};

class TestB {
public:
    TestClass* p;

    TestB(TestClass* p) { std::cout << "TestB(" << p->X << ")" << std::endl; }

    static void Print(TestClass* p) { std::cout << "TestB::Print(" << p->X << ")" << std::endl; }

    TestClass* GetP() { return p; }
};

UsingCppType(TestClass);
UsingCppType(TestB);

namespace jse {
namespace fs = std::filesystem;

JavaScriptPluginLoader::JavaScriptPluginLoader(endstone::Server& server) : PluginLoader(server) {}

std::vector<endstone::Plugin*> JavaScriptPluginLoader::loadPlugins(const std::string& directory) {
    std::vector<endstone::Plugin*> plugins;

    // NodeJs & package.json
    try {
        fs::path dir(directory);
        if (!fs::exists(dir)) {
            return plugins;
        }

        auto dirs = fs::directory_iterator(dir);
        for (const auto& entry : dirs) {
            // find package.json
            if (!entry.is_directory()) {
                continue;
            }
            fs::path packageJsonPath = entry.path() / "package.json";
            if (!fs::exists(packageJsonPath)) {
                GetEntry()->getLogger().debug(fmt::format("Skipping non-package.json file: {}", entry.path()));
                continue;
            }

            // load package.json
            try {
                std::ifstream file(packageJsonPath);
                auto          package = nlohmann::json::parse(file);
                file.close();

                // try to load plugin
                if (!package.contains("main")) {
                    GetEntry()->getLogger().error(fmt::format("No main field in package.json: {}", entry.path()));
                    continue;
                }

                auto main = package["main"];
                if (!main.is_string()) {
                    GetEntry()->getLogger().error(
                        fmt::format("Main field in package.json is not a string: {}", entry.path())
                    );
                    continue;
                }

                fs::path mainPath = entry.path() / main;
                if (!fs::exists(mainPath) || mainPath.extension() != ".js") {
                    GetEntry()->getLogger().error(fmt::format("Main file does not exist: {}", mainPath));
                    continue;
                }

                // load plugin
                if (auto plugin = loadPlugin(mainPath)) {
                    plugins.push_back(plugin);
                }
            } catch (nlohmann::json::parse_error& e) {
                GetEntry()->getLogger().error(fmt::format("Error occurred while parsing package.json: {}", e.what()));
                continue;
            } catch (std::exception& e) {
                GetEntry()->getLogger().error(fmt::format("Error occurred while parsing package.json: {}", e.what()));
                continue;
            } catch (...) {
                GetEntry()->getLogger().error(fmt::format("Error occurred while parsing package.json: Unknown error"));
                continue;
            }
        }

    } catch (std::exception& e) {
        GetEntry()->getLogger().error(
            fmt::format("Error occurred while loading plugins from '{}': {}", directory, e.what())
        );
    } catch (...) {
        GetEntry()->getLogger().error(
            fmt::format("Error occurred while loading plugins from '{}': Unknown error", directory)
        );
    }

    return plugins;
}

std::string FixWinPath(const std::string& path) {
    // 将路径中的反斜杠替换为正斜杠
    std::string fixedPath = path;
    std::replace(fixedPath.begin(), fixedPath.end(), '\\', '/');
    return fixedPath;
}


endstone::Plugin* JavaScriptPluginLoader::loadPlugin(const fs::path& file) {
    // NodeJS
    auto& nl  = NodeHelper::getInstance();
    auto  val = nl.newEngine(file.string());
    if (!val) {
        GetEntry()->getLogger().error(fmt::format("Failed to load plugin '{}'", file.string()));
        return nullptr;
    }
    GetEntry()->getLogger().debug(fmt::format("Loaded plugin '{}'", file.string()));


    Isolate*     isolate = val->isolate;
    Environment* env     = val->env;
    {
        Locker         locker(isolate);
        Isolate::Scope isolate_scope(isolate);
        HandleScope    handle_scope(isolate);
        Context::Scope context_scope(val->setup->context());

        // 启用ESM系统
        v8::MaybeLocal<Value> local = node::LoadEnvironment(
            env,
            "const publicRequire = require('module').createRequire(process.cwd() + '/');"
            "globalThis.require = publicRequire;"
            // 设置模块加载器
            "import('node:module').then(async (module) => {"
            "  const { pathToFileURL } = await import('node:url');"
            "  const filePath = pathToFileURL('"
                + FixWinPath(file.string())
                + "').href;"
                  "  await import(filePath);"
                  "});"
        );

        if (local.IsEmpty()) {
            GetEntry()->getLogger().error(fmt::format("Failed to load plugin '{}'", file.string()));
            nl.destroyEngine(val->pluginName);
            return nullptr;
        }

        int exit_code = node::SpinEventLoop(env).FromMaybe(1);

        auto                     context = val->setup->context();
        puerts::FCppObjectMapper cppObjectMapper;
        cppObjectMapper.Initialize(isolate, context);
        isolate->SetData(MAPPER_ISOLATE_DATA_POS, static_cast<puerts::ICppObjectMapper*>(&cppObjectMapper));

        context->Global()
            ->Set(
                context,
                v8::String::NewFromUtf8(isolate, "loadCppType").ToLocalChecked(),
                v8::FunctionTemplate::New(
                    isolate,
                    [](const v8::FunctionCallbackInfo<v8::Value>& info) {
                        auto pom =
                            static_cast<puerts::FCppObjectMapper*>((v8::Local<v8::External>::Cast(info.Data()))->Value()
                            );
                        pom->LoadCppType(info);
                    },
                    v8::External::New(isolate, &cppObjectMapper)
                )
                    ->GetFunction(context)
                    .ToLocalChecked()
            )
            .Check();

        puerts::DefineClass<TestClass>()
            .Constructor<int>()
            .Function("Print", MakeFunction(&TestClass::Print))
            .Property("X", MakeProperty(&TestClass::X))
            .Method("Add", MakeFunction(&TestClass::Add))
            .Register();

        puerts::DefineClass<TestB>()
            .Constructor<TestClass*>()
            .Function("Print", MakeFunction(&TestB::Print))
            .Method("GetP", MakeFunction(&TestB::GetP))
            .Register();

        {
            // Native new TestClass to ScriptEngine
            auto testInstance = new TestClass(42); // 创建实例

            // 将C++对象包装为JS对象
            auto jsTestInstance = puerts::DataTransfer::FindOrAddCData(
                isolate,
                context,
                puerts::StaticTypeId<TestClass>::get(),
                testInstance,
                false // 这里false表示不是指针传递
            );

            // 创建一个全局函数来获取这个实例
            auto getNativeTestClass = v8::FunctionTemplate::New(
                isolate,
                [](const v8::FunctionCallbackInfo<v8::Value>& info) {
                    auto isolate      = info.GetIsolate();
                    auto context      = isolate->GetCurrentContext();
                    auto testInstance = puerts::DataTransfer::FindOrAddCData(
                        isolate,
                        context,
                        puerts::StaticTypeId<TestClass>::get(),
                        info.Data().As<v8::External>()->Value(),
                        false
                    );
                    info.GetReturnValue().Set(testInstance);
                },
                v8::External::New(isolate, testInstance) // 将实例作为函数数据传递
            );

            // 将函数设置为全局对象的属性
            context->Global()
                ->Set(
                    context,
                    v8::String::NewFromUtf8(isolate, "getNativeTestClass").ToLocalChecked(),
                    getNativeTestClass->GetFunction(context).ToLocalChecked()
                )
                .Check();
        }


        // 加载文件
        const char* csource;
        if (!std::filesystem::exists(file)) {
            GetEntry()->getLogger().error(fmt::format("Failed to load plugin '{}'", file.string()));
            nl.destroyEngine(val->pluginName);
            return nullptr;
        }
        std::ifstream t(file);
        std::string   str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        csource = str.c_str();
        if (csource == nullptr) {
            GetEntry()->getLogger().error(fmt::format("Failed to load plugin '{}'", file.string()));
            nl.destroyEngine(val->pluginName);
            return nullptr;
        }

        // Create a string containing the JavaScript source code.
        v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, csource).ToLocalChecked();

        // Compile the source code.
        v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();

        // Run the script
        auto _unused = script->Run(context);

        // cppObjectMapper.UnInitialize(isolate);

        // node::Stop(env);
        val->uvThread = std::jthread([isolate, event_loop{val->setup->event_loop()}](std::stop_token stop_token) {
            Locker         lock{isolate};
            Isolate::Scope isolate_scope{isolate};
            while (!stop_token.stop_requested()) {
                uv_run(event_loop, UV_RUN_NOWAIT);
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 等待100毫秒
            }
        });
    }


    return nullptr;
}

void JavaScriptPluginLoader::enablePlugin(endstone::Plugin& plugin) const { PluginLoader::enablePlugin(plugin); }
void JavaScriptPluginLoader::disablePlugin(endstone::Plugin& plugin) const { PluginLoader::disablePlugin(plugin); }


} // namespace jse
