#include "JavaScriptPluginLoader.h"
#include "Engine/EngineManager.h"
#include "Engine/EngineSelfData.h"
#include "Engine/Using.h"
#include "Entry.h"
#include "JavaScriptPlugin.h"
#include "endstone/detail/server.h"
#include "fmt/format.h"
#include <filesystem>

#include "API/APIHelper.h"

namespace jse {
namespace fs = std::filesystem;

JavaScriptPluginLoader::JavaScriptPluginLoader(endstone::Server& server) : PluginLoader(server) {}

std::vector<endstone::Plugin*> JavaScriptPluginLoader::loadPlugins(const std::string& directory) {
    std::vector<endstone::Plugin*> plugins;

    try {
        fs::path dir(directory);
        if (!fs::exists(dir)) {
            return plugins;
        }

        // 遍历目录查找.js文件
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.path().extension() == ".js") {
                if (auto plugin = loadPlugin(entry.path())) {
                    plugins.push_back(plugin);
                }
            }
        }

    } catch (const std::exception& e) {
        GetEntry()->getLogger().error(
            fmt::format("Error occurred while loading plugins from '{}': {}", directory, e.what())
        );
    }

    return plugins;
}

endstone::Plugin* JavaScriptPluginLoader::loadPlugin(const fs::path& file) {
    try {
        // 创建新的JS引擎实例
        auto& engineManager = EngineManager::getInstance();
        auto* engine        = engineManager.createEngine();
        if (!engine) {
            GetEntry()->getLogger().error(fmt::format("Failed to create JS engine for plugin: {}", file.string()));
            return nullptr;
        }
        EngineScope scope(engine); // 进入引擎作用域
        auto        data            = ENGINE_SELF_DATA();
        data->mJSE_Plugin.mFileName = fs::path(file).filename().string(); // 设置插件文件名

        // 加载JS文件
        engine->loadFile(file.string());

        // 创建插件实例
        auto* plugin = new JavaScriptPlugin(
            data->mEngineId,
            data->mJSE_Plugin.getName(),
            data->mJSE_Plugin.getVersion(),
            data->mJSE_Plugin.getDescription()
        );
        data->mPlugin = plugin; // 将插件实例保存到引擎数据中

        plugin->onLoad(); // 调用插件的onLoad回调

        return plugin;
    } catch (const script::Exception& e) {
        GetEntry()->getLogger().error(fmt::format("Failed to load plugin '{}': {}", file.string(), e.stacktrace()));
        return nullptr;
    } catch (const std::exception& e) {
        GetEntry()->getLogger().error(fmt::format("Failed to load plugin '{}': {}", file.string(), e.what()));
        return nullptr;
    } catch (...) {
        GetEntry()->getLogger().error(fmt::format("Failed to load plugin '{}': Unknown error", file.string()));
        return nullptr;
    }
}

void JavaScriptPluginLoader::enablePlugin(endstone::Plugin& plugin) const {
    try {
        PluginLoader::enablePlugin(plugin);
    }
    CatchNotReturn;
}
void JavaScriptPluginLoader::disablePlugin(endstone::Plugin& plugin) const {
    try {
        PluginLoader::disablePlugin(plugin);
    }
    CatchNotReturn;
}

} // namespace jse
