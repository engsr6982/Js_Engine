#include "JavaScriptPluginLoader.h"
#include "Entry.h"
#include "JavaScriptPlugin.h"
#include "Node/NodeHelper.h"
#include "Utils/Util.h"
#include "endstone/detail/server.h"
#include "endstone/permissions/permission_default.h"
#include "endstone/plugin/plugin_load_order.h"
#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>


namespace jse {
namespace fs = std::filesystem;

JavaScriptPluginLoader::JavaScriptPluginLoader(endstone::Server& server) : PluginLoader(server) {}

std::vector<std::string> JavaScriptPluginLoader::getPluginFileFilters() const {
    return {}; // 因为 NodeJs 插件为 floder，无需过滤
}


endstone::PluginLoadOrder ParseLoadOrder(std::string const& loadOrder) {
    try {
        return magic_enum::enum_cast<endstone::PluginLoadOrder>(loadOrder).value();
    } catch (...) {
        return endstone::PluginLoadOrder::PostWorld;
    }
}
endstone::PermissionDefault ParseDefaultPermission(std::string const& defaultPermission) {
    try {
        return magic_enum::enum_cast<endstone::PermissionDefault>(defaultPermission).value();
    } catch (...) {
        return endstone::PermissionDefault::Operator;
    }
}


endstone::Plugin* JavaScriptPluginLoader::loadPlugin(std::string file) {
    // NodeJS
    auto& helper = NodeHelper::getInstance();
    auto  engine = helper.newEngine(file);
    if (!engine) {
        GetEntry()->getLogger().error(fmt::format("Failed to load plugin '{}'", file));
        return nullptr;
    }
    GetEntry()->getLogger().debug(fmt::format("Loaded plugin '{}'", file));


    Isolate*     isolate = engine->isolate();
    Environment* env     = engine->env();
    auto         context = engine->setup->context();

    // 上锁并进入作用域
    Locker         locker(isolate);
    Isolate::Scope isolate_scope(isolate);
    HandleScope    handle_scope(isolate);
    Context::Scope context_scope(context);

    if (!engine->loadPluginCode(file)) {
        GetEntry()->getLogger().error(fmt::format("Failed to load plugin code '{}'", file));
        return nullptr;
    }

    // 获取插件注册信息
    auto pluginInfo = engine->pluginInfo.Get(isolate);
    if (pluginInfo.IsEmpty()) {
        GetEntry()->getLogger().error(fmt::format("Plugin '{}' did not register properly", file));
        return nullptr;
    }

    // 创建插件实例
    auto plugin = new JavaScriptPlugin(
        engine->engineId,
        TryParseString("name", pluginInfo, isolate, context),
        TryParseString("version", pluginInfo, isolate, context),
        TryParseString("description", pluginInfo, isolate, context),
        ParseLoadOrder(TryParseString("load", pluginInfo, isolate, context)),
        TryParseStringArray("authors", pluginInfo, isolate, context),
        TryParseStringArray("contributors", pluginInfo, isolate, context),
        TryParseString("website", pluginInfo, isolate, context),
        TryParseString("prefix", pluginInfo, isolate, context),
        TryParseStringArray("provides", pluginInfo, isolate, context),
        TryParseStringArray("depend", pluginInfo, isolate, context),
        TryParseStringArray("soft_depend", pluginInfo, isolate, context),
        TryParseStringArray("load_before", pluginInfo, isolate, context),
        ParseDefaultPermission(TryParseString("default_permission", pluginInfo, isolate, context)),
        std::nullopt, // TODO: commands
        std::nullopt  // TODO: permissions
    );

    // 保存插件实例到引擎
    engine->plugin = plugin;
    engine->initUvLoop(); // 初始化 uv loop

    return plugin;


    // {
    //     // Native new TestClass to ScriptEngine
    //     auto testInstance = new TestClass(42); // 创建实例
    //     // 将C++对象包装为JS对象
    //     auto jsTestInstance = puerts::DataTransfer::FindOrAddCData(
    //         isolate,
    //         context,
    //         puerts::StaticTypeId<TestClass>::get(),
    //         testInstance,
    //         false // 这里false表示不是指针传递
    //     );
    //     // 创建一个全局函数来获取这个实例
    //     auto getNativeTestClass = v8::FunctionTemplate::New(
    //         isolate,
    //         [](const v8::FunctionCallbackInfo<v8::Value>& info) {
    //             auto isolate      = info.GetIsolate();
    //             auto context      = isolate->GetCurrentContext();
    //             auto testInstance = puerts::DataTransfer::FindOrAddCData(
    //                 isolate,
    //                 context,
    //                 puerts::StaticTypeId<TestClass>::get(),
    //                 info.Data().As<v8::External>()->Value(),
    //                 false
    //             );
    //             info.GetReturnValue().Set(testInstance);
    //         },
    //         v8::External::New(isolate, testInstance) // 将实例作为函数数据传递
    //     );
    //     // 将函数设置为全局对象的属性
    //     context->Global()
    //         ->Set(
    //             context,
    //             v8::String::NewFromUtf8(isolate, "getNativeTestClass").ToLocalChecked(),
    //             getNativeTestClass->GetFunction(context).ToLocalChecked()
    //         )
    //         .Check();
    // }

    // 注册类型
    // puerts::DefineClass<TestClass>()
    //     .Constructor<int>()
    //     .Function("Print", MakeFunction(&TestClass::Print))
    //     .Property("X", MakeProperty(&TestClass::X))
    //     .Method("Add", MakeFunction(&TestClass::Add))
    //     .Register();
    // puerts::DefineClass<TestB>()
    //     .Constructor<TestClass*>()
    //     .Function("Print", MakeFunction(&TestB::Print))
    //     .Method("GetP", MakeFunction(&TestB::GetP))
    //     .Register();
}


std::vector<std::string> JavaScriptPluginLoader::filterPlugins(const std::filesystem::path& directory) {
    std::vector<std::string> plugins;
    // NodeJs & package.json
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

            plugins.push_back(mainPath.string());
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

    return plugins;
}


} // namespace jse
