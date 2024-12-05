#include "JavaScriptPluginLoader.h"
#include "Binding.hpp"
#include "CppObjectMapper.h"
#include "DataTransfer.h"
#include "Entry.h"
#include "JavaScriptPlugin.h"
#include "NodeHelper.h"
#include "ScriptBackend.hpp"
#include "TypeInfo.hpp"
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
#include <memory>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>


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

namespace puerts {
template <typename T>
struct ScriptTypeName<std::vector<T*>> {
    static constexpr auto value() { return internal::Literal("Array"); }
};

template <typename T>
struct ScriptTypeName<std::vector<T*>*> {
    static constexpr auto value() { return internal::Literal("Array"); }
};

// 实现Converter
namespace v8_impl {
template <typename T>
struct Converter<std::vector<T*>> {
    static v8::Local<v8::Value> toScript(v8::Local<v8::Context> context, const std::vector<T*>& vec) {
        v8::Isolate*         isolate = context->GetIsolate();
        v8::Local<v8::Array> jsArray = v8::Array::New(isolate, vec.size());
        for (size_t i = 0; i < vec.size(); ++i) {
            jsArray->Set(context, i, Converter<T*>::toScript(context, vec[i])).Check();
        }
        return jsArray;
    }

    static std::vector<T*> toCpp(v8::Local<v8::Context> context, v8::Local<v8::Value> value) {
        std::vector<T*> vec;
        if (value->IsArray()) {
            v8::Local<v8::Array> jsArray = value.As<v8::Array>();
            for (uint32_t i = 0; i < jsArray->Length(); ++i) {
                vec.push_back(Converter<T*>::toCpp(context, jsArray->Get(context, i).ToLocalChecked()));
            }
        }
        return vec;
    }
};

template <typename T>
struct Converter<std::vector<T*>*> {
    static v8::Local<v8::Value> toScript(v8::Local<v8::Context> context, std::vector<T*>* vec) {
        if (!vec) return v8::Null(context->GetIsolate());
        return Converter<std::vector<T*>>::toScript(context, *vec);
    }

    static std::vector<T*>* toCpp(v8::Local<v8::Context> context, v8::Local<v8::Value> value) {
        if (value->IsNull() || value->IsUndefined()) return nullptr;
        auto result = new std::vector<T*>(Converter<std::vector<T*>>::toCpp(context, value));
        return result;
    }
};
} // namespace v8_impl
} // namespace puerts
template <typename T>
v8::Local<v8::Value> Convert(Isolate* isolate, v8::Local<v8::Context> context, T* value) {
    return puerts::DataTransfer::FindOrAddCData(isolate, context, puerts::StaticTypeId<T>::get(), value, false);
}
template <typename T>
v8::Local<v8::Value> Convert(Isolate* isolate, v8::Local<v8::Context> context, T& value) {
    return puerts::DataTransfer::FindOrAddCData(isolate, context, puerts::StaticTypeId<T>::get(), value, false);
}

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

        auto context = val->setup->context();

        // 先设置好所有的API绑定
        auto cppObjectMapper = new puerts::FCppObjectMapper();
        cppObjectMapper->Initialize(isolate, context);
        isolate->SetData(MAPPER_ISOLATE_DATA_POS, static_cast<puerts::ICppObjectMapper*>(cppObjectMapper));

        // 绑定loadCppType
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

        // 注册类型
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


        // 然后启动ESM系统
        // clang-format off
        v8::MaybeLocal<Value> local = node::LoadEnvironment(
            env,
            R"(
                const publicRequire = require('module').createRequire(process.cwd() + '/');
                globalThis.require = publicRequire;

                import('node:module').then(async (module) => {
                    const { pathToFileURL } = await import('node:url');
                    const filePath = pathToFileURL(')" + FixWinPath(file.string()) + R"(').href;
                    await import(filePath);
                });
            )"
        );
        // clang-format on

        if (local.IsEmpty()) {
            GetEntry()->getLogger().error(fmt::format("Failed to load plugin '{}'", file.string()));
            nl.destroyEngine(val->pluginName);
            return nullptr;
        }

        int exit_code = node::SpinEventLoop(env).FromMaybe(1);

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
