#include "Entry.h"
#include "Loader/JavaScriptPlugin.h"
#include "Loader/JavaScriptPluginLoader.h"
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
    getLogger().info("Js_Engine loading...");
    getLogger().info("Configuring nodejs...");

    mPlatform = node::MultiIsolatePlatform::Create(4);
    V8::InitializePlatform(mPlatform.get());
    V8::SetFlagsFromString("--no-freeze-flags-after-init"); // 禁止在初始化后修改标志
    V8::Initialize();

    static int               argc   = 1;
    static char*             argv[] = {(char*)"bedrock_server.exe", nullptr}; // 至少需要程序名
    auto                     u_argv = uv_setup_args(argc, argv);
    std::vector<std::string> args(argv, u_argv + argc);
    auto                     res = node::InitializeOncePerProcess(
        args,
        {
            node::ProcessInitializationFlags::kNoDefaultSignalHandling, // 不注册默认信号处理器
            node::ProcessInitializationFlags::kNoStdioInitialization,     // 不初始化标准IO
            node::ProcessInitializationFlags::kNoInitializeV8,            // 不初始化 V8
            node::ProcessInitializationFlags::kNoInitializeNodeV8Platform // 自己管理V8平台
        }
    );
    if (res->exit_code() != 0) {
        for (std::string const& err : res->errors()) {
            getLogger().error(err);
        }
        getLogger().error("Failed to initialize nodejs, exiting...");
        exit(res->exit_code()); // 退出程序
    }


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

    node::TearDownOncePerProcess(); // 清理nodejs
    V8::Dispose();                  // 清理v8
    V8::DisposePlatform();          // 清理platform

    getLogger().info("Js_Engine disabled");
}
