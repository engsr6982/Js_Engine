#include "JavaScriptPluginLoader.h"
#include "Entry.h"
#include "JavaScriptPlugin.h"
#include "endstone/detail/server.h"
#include "fmt/format.h"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include <filesystem>


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

        auto dirs = fs::recursive_directory_iterator(dir);
        for (const auto& entry : dirs) {
            // find package.json
            if (entry.path().filename() != "package.json") {
                continue;
            }

            // load package.json
            try {
                nlohmann::json package;
                std::ifstream  file(entry.path());
                file >> package;
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

endstone::Plugin* JavaScriptPluginLoader::loadPlugin(const fs::path& file) {
    // try {
    //     // 创建新的JS引擎实例
    //     auto& engineManager = EngineManager::getInstance();
    //     auto* engine        = engineManager.createEngine();
    //     if (!engine) {
    //         GetEntry()->getLogger().error(fmt::format("Failed to create JS engine for plugin: {}",
    //         file.string())); return nullptr;
    //     }
    //     EngineScope scope(engine); // 进入引擎作用域
    //     auto        data            = ENGINE_SELF_DATA();
    //     data->mJSE_Plugin.mFileName = fs::path(file).filename().string(); // 设置插件文件名

    //     // 加载JS文件
    //     engine->loadFile(file.string());

    //     // 创建插件实例
    //     auto* plugin = new JavaScriptPlugin(
    //         data->mEngineId,
    //         data->mJSE_Plugin.getName(),
    //         data->mJSE_Plugin.getVersion(),
    //         data->mJSE_Plugin.getDescription()
    //     );
    //     data->mPlugin = plugin; // 将插件实例保存到引擎数据中

    //     plugin->onLoad(); // 调用插件的onLoad回调

    //     return plugin;
    // } catch (const script::Exception& e) {
    //     GetEntry()->getLogger().error(fmt::format("Failed to load plugin '{}': {}", file.string(),
    //     e.stacktrace())); return nullptr;
    // } catch (const std::exception& e) {
    //     GetEntry()->getLogger().error(fmt::format("Failed to load plugin '{}': {}", file.string(), e.what()));
    //     return nullptr;
    // } catch (...) {
    //     GetEntry()->getLogger().error(fmt::format("Failed to load plugin '{}': Unknown error", file.string()));
    //     return nullptr;
    // }

    return nullptr;
}

void JavaScriptPluginLoader::enablePlugin(endstone::Plugin& plugin) const { PluginLoader::enablePlugin(plugin); }
void JavaScriptPluginLoader::disablePlugin(endstone::Plugin& plugin) const { PluginLoader::disablePlugin(plugin); }


} // namespace jse
