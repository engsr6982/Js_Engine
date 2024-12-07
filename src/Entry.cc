#include "Entry.h"
#include "Loader/JavaScriptPlugin.h"
#include "Loader/JavaScriptPluginLoader.h"
#include "Node/NodeHelper.h"
#include "Node/UsingV8.h"
#include "endstone/detail/logger_factory.h"
#include "endstone/detail/plugin/plugin_manager.h"
#include "endstone/detail/server.h"
#include "endstone/plugin/plugin_manager.h"
#include "node.h"
#include <filesystem>
#include <memory>
#include <thread>
#include <utility>


ENDSTONE_PLUGIN("js_engine", "0.1.0", Entry) {
    description  = "JavaScript Engine";
    authors      = {"engsr6982"};
    contributors = {"engsr6982"};
    website      = "https://github.com/engsr6982/Js_Engine";
}


Entry* __Entry = nullptr;
Entry* GetEntry() { return __Entry; }


using endstone::detail::EndstoneServer;
void Entry::onLoad() {
    __Entry = this;
    getLogger().info("Js_Engine loading...");

#ifdef DEBUG
    getLogger().setLevel(endstone::Logger::Debug);
    getLogger().info("Waiting for VC debugger attach...");
    std::this_thread::sleep_for(std::chrono::seconds(10));
#endif

    getLogger().info("Initialize Node.js and V8 platform...");
    jse::NodeHelper::getInstance().initNodeJs();

    getLogger().info("Loading javascript plugin...");
    getServer().getPluginManager().registerLoader(std::make_unique<jse::JavaScriptPluginLoader>(getServer()));

    auto dirs = jse::JavaScriptPluginLoader::filterPlugins(fs::current_path() / "plugins");
    getServer().getPluginManager().loadPlugins(std::move(dirs));
}

void Entry::onEnable() { getLogger().info("Js_Engine enabled"); }

void Entry::onDisable() {
    __Entry = nullptr;

    jse::NodeHelper::getInstance().shutdownNodeJs();

    getLogger().info("Js_Engine disabled");
}
