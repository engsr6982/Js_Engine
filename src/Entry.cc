#include "Entry.h"
#include "Loader/JavaScriptPlugin.h"
#include "Loader/JavaScriptPluginLoader.h"
#include "NodeHelper.h"
#include "Using.h"
#include "endstone/detail/logger_factory.h"
#include "endstone/detail/plugin/plugin_manager.h"
#include "endstone/detail/server.h"
#include "endstone/plugin/plugin_manager.h"
#include "node.h"
#include <filesystem>
#include <memory>
#include <utility>


ENDSTONE_PLUGIN("js_engine", "0.1.0", Entry) { description = "JavaScript Engine"; }


// Entry *__Entry = new Entry();
Entry* __Entry = nullptr;
Entry* GetEntry() { return __Entry; }


using endstone::detail::EndstoneServer;
void Entry::onLoad() {
    __Entry = this;

#ifdef DEBUG
    getLogger().setLevel(endstone::Logger::Debug);
#endif

    getLogger().info("Js_Engine loading...");

    getLogger().info("Configuring nodejs...");
    jse::NodeHelper::getInstance().initNodeJs();

    getLogger().info("Loading javascript plugin...");
    auto loader = std::make_unique<jse::JavaScriptPluginLoader>(getServer());

    auto& manager = static_cast<endstone::detail::EndstonePluginManager&>(getServer().getPluginManager());
    auto  plugins = loader->loadPlugins((fs::current_path() / "plugins").string());
    for (auto const& plugin : plugins) {
        if (!plugin) {
            getLogger().error("Failed to load plugin: " + plugin->getName());
            continue;
        }
        auto name        = plugin->getDescription().getName();
        plugin->loader_  = loader.get();
        plugin->server_  = &manager.server_;
        auto plugin_name = plugin->getDescription().getName();
        auto prefix      = plugin->getDescription().getPrefix();
        if (prefix.empty()) {
            prefix = plugin_name;
        }
        // plugin->logger_      = &endstone::detail::LoggerFactory::getLogger(prefix);
        plugin->logger_      = &getLogger(); // TODO: fix this
        plugin->data_folder_ = fs::current_path() / "plugins" / plugin_name;
        manager.plugins_.push_back(plugin);
        manager.lookup_names_[name] = plugin;
    }

    getServer().getPluginManager().registerLoader(std::move(loader)); // 移交所有权
}

void Entry::onEnable() { getLogger().info("Js_Engine enabled"); }

void Entry::onDisable() {
    __Entry = nullptr;

    jse::NodeHelper::getInstance().destroyNodeJs();

    getLogger().info("Js_Engine disabled");
}
