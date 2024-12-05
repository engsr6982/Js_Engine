#pragma once
#include "Entry.h"
#include "Using.h"
#include "node.h"
#include "uv.h"
#include "v8-context.h"
#include <filesystem>
#include <string>
#include <thread>
#include <unordered_map>

namespace jse {

namespace fs = std::filesystem;

// 管理NodeJs引擎 和每个插件独立的Isolate
class NodeHelper {
public:
    struct Engine {
        std::string path;
        std::string pluginName;

        std::unique_ptr<CommonEnvironmentSetup> setup;
        Isolate*                                isolate;
        Environment*                            env;

        std::jthread uvThread;
    };

public:
    std::unique_ptr<MultiIsolatePlatform> mPlatform;

    std::unordered_map<std::string, Engine> mNodeEngines; // 插件名 -> NodeJs引擎

    int const _argc    = 2;
    char*     _argv[2] = {
        (char*)"bedrock_server.exe",
        (char*)"",
        // (char*)"--inspect-brk=9229"
    };

public:
    static NodeHelper& getInstance() {
        static NodeHelper instance;
        return instance;
    }

    void initNodeJs() {
        mPlatform = node::MultiIsolatePlatform::Create(std::thread::hardware_concurrency());
        V8::InitializePlatform(mPlatform.get());
        V8::SetFlagsFromString("--no-freeze-flags-after-init"); // 禁止在初始化后修改标志
        V8::Initialize();

        auto                     u_argv = uv_setup_args(_argc, _argv);
        std::vector<std::string> args(_argv, u_argv + _argc);
        auto                     res = node::InitializeOncePerProcess(
            args,
            {
                node::ProcessInitializationFlags::kNoInitializeV8,            // 不初始化 V8
                node::ProcessInitializationFlags::kNoInitializeNodeV8Platform // 自己管理V8平台
            }
        );
        if (res->exit_code() != 0) {
            for (std::string const& err : res->errors()) {
                GetEntry()->getLogger().error(err);
            }
            GetEntry()->getLogger().error("Failed to initialize nodejs, exiting...");
            exit(res->exit_code()); // 退出程序
        }
    }

    MultiIsolatePlatform* getPlatform() { return mPlatform.get(); }

    Engine* newEngine(std::string const& file) {
        fs::path    path       = file;
        std::string pluginName = path.filename().string();

        // 检查是否已存在
        if (mNodeEngines.contains(pluginName)) {
            throw std::runtime_error("Plugin " + pluginName + " already exists");
        }

        Engine engine;
        engine.pluginName = pluginName;
        engine.path       = file;

        // 将参数转换为 vector<string>
        std::vector<std::string> args;
        std::vector<std::string> exec_args;
        for (int i = 0; i < _argc; i++) {
            args.emplace_back(_argv[i]);
        }

        // 使用 CommonEnvironmentSetup 来创建环境
        static std::vector<std::string> errors;
        engine.setup = CommonEnvironmentSetup::Create(getPlatform(), &errors, args, exec_args);

        if (!engine.setup) {
            for (std::string const& err : errors) {
                GetEntry()->getLogger().error(err);
            }
            errors.clear();
            return nullptr;
        }

        engine.isolate = engine.setup->isolate();
        engine.env     = engine.setup->env();

        // 存储并返回引擎实例
        auto [it, success] = mNodeEngines.emplace(pluginName, std::move(engine));
        return &it->second;
    }

    bool destroyEngine(std::string const& pluginName) {
        auto it = mNodeEngines.find(pluginName);
        if (it == mNodeEngines.end()) {
            return false;
        }

        auto& engine = it->second;

        if (engine.isolate) {
            engine.isolate->Dispose();
        }

        mNodeEngines.erase(it);
        return true;
    }

    void destroyNodeJs() {
        // 先停止所有环境的事件循环
        for (auto& [name, engine] : mNodeEngines) {
            if (engine.env) {
                engine.uvThread.request_stop(); // 停止 uv 事件循环
                uv_stop(engine.setup->event_loop());
                node::Stop(engine.env);
            }
        }

        // 清理每个引擎
        for (auto& [name, engine] : mNodeEngines) {
            if (engine.setup) {
                engine.env     = nullptr; // 清空引用
                engine.isolate = nullptr; // 清空引用，因为 setup 会处理它们
                engine.setup.reset();     // CommonEnvironmentSetup 会正确清理 Environment 和 Isolate
            }
        }
        mNodeEngines.clear();

        // 清理全局资源
        node::TearDownOncePerProcess();
        V8::Dispose();
        V8::DisposePlatform();
        mPlatform.reset();
    }
};


} // namespace jse