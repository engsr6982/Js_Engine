#pragma once
#include "API/JSEAPI.h"
#include "Entry.h"
#include "Loader/JavaScriptPlugin.h"
#include "Node/UsingV8.h"
#include "Utils/Util.h"
#include "endstone/scheduler/scheduler.h"
#include "node.h"
#include "v8-object.h"
#include "v8-persistent-handle.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

namespace jse {

namespace fs = std::filesystem;

#define V8_ENGINE_ID "__V8_ENGINE_ID__"


class NodeHelper {
public:
    struct Engine {
    public:
        uint64_t    engineId;    // 引擎id
        std::string fileName;    // 插件文件名 (main)
        std::string packagePath; // 插件路径

        std::unique_ptr<CommonEnvironmentSetup>   setup{nullptr};
        std::unique_ptr<puerts::FCppObjectMapper> fCppObjectMapper{nullptr};

        std::shared_ptr<endstone::Task> uvLoopTask{nullptr};

        JavaScriptPlugin*      plugin{nullptr};
        v8::Global<v8::Object> pluginInfo; // 插件注册信息

    public:
        Isolate*               isolate() { return setup->isolate(); }
        Environment*           env() { return setup->env(); }
        v8::Local<v8::Context> context() { return setup->context(); }
        JavaScriptPlugin*      getPlugin() { return plugin; }

    public:
        bool callOnLoad() {
            try {
                auto isolate_ = isolate();
                auto context_ = context();

                auto infoObj = pluginInfo.Get(isolate_);
                auto onLoad  = infoObj->Get(context_, v8::String::NewFromUtf8(isolate_, "onLoad").ToLocalChecked())
                                  .ToLocalChecked();

                if (onLoad->IsFunction()) {
                    auto func = onLoad.As<v8::Function>();
                    func->Call(context_, infoObj, 0, nullptr).ToLocalChecked();
                    return true;
                }
                return false;
            } catch (...) {
                return false;
            }
        }

        bool callOnEnable() {
            try {
                auto isolate_ = isolate();
                auto context_ = context();

                auto infoObj = pluginInfo.Get(isolate_);
                auto onLoad  = infoObj->Get(context_, v8::String::NewFromUtf8(isolate_, "onEnable").ToLocalChecked())
                                  .ToLocalChecked();

                if (onLoad->IsFunction()) {
                    auto func = onLoad.As<v8::Function>();
                    func->Call(context_, infoObj, 0, nullptr).ToLocalChecked();
                    return true;
                }
                return false;
            } catch (...) {
                return false;
            }
        }

        bool callOnDisable() {
            try {
                auto isolate_ = isolate();
                auto context_ = context();

                auto infoObj = pluginInfo.Get(isolate_);
                auto onLoad  = infoObj->Get(context_, v8::String::NewFromUtf8(isolate_, "onDisable").ToLocalChecked())
                                  .ToLocalChecked();

                if (onLoad->IsFunction()) {
                    auto func = onLoad.As<v8::Function>();
                    func->Call(context_, infoObj, 0, nullptr).ToLocalChecked();
                    return true;
                }
                return false;
            } catch (...) {
                return false;
            }
        }

    private:
        bool _initESM(const fs::path& file) {
            // 启动ESM系统
            // clang-format off
            v8::MaybeLocal<Value> local = node::LoadEnvironment(
                env(),
                R"(
                    const publicRequire = require('module').createRequire(process.cwd() + '/');
                    globalThis.require = publicRequire;

                    import('node:module').then(async (module) => {
                        const { pathToFileURL } = await import('node:url');
                        const filePath = pathToFileURL(')" + ReplaceWinPath(file.string()) + R"(').href;
                        await import(filePath);
                    });
                )"
            );
            // clang-format on

            if (local.IsEmpty()) {
                GetEntry()->getLogger().error("Failed to init ESM: {}", file.string());
                return false;
            }
            return true;
        }

        bool _bindNativeAPI() {
            try {
                // 初始化 PuerTs
                if (!fCppObjectMapper) {
                    fCppObjectMapper = std::make_unique<puerts::FCppObjectMapper>();
                }
                fCppObjectMapper->Initialize(isolate(), context());
                isolate()->SetData(
                    MAPPER_ISOLATE_DATA_POS,
                    static_cast<puerts::ICppObjectMapper*>(fCppObjectMapper.get())
                );

                // 绑定 loadNativeType
                // 挂载到全局对象，作为基础 API
                {
                    context()
                        ->Global()
                        ->Set(
                            context(),
                            v8::String::NewFromUtf8(isolate(), "loadNativeType").ToLocalChecked(),
                            v8::FunctionTemplate::New(
                                isolate(),
                                [](const v8::FunctionCallbackInfo<v8::Value>& info) {
                                    auto pom = static_cast<puerts::FCppObjectMapper*>(
                                        (v8::Local<v8::External>::Cast(info.Data()))->Value()
                                    );
                                    pom->LoadCppType(info);
                                },
                                v8::External::New(isolate(), fCppObjectMapper.get())
                            )
                                ->GetFunction(context())
                                .ToLocalChecked()
                        )
                        .Check();
                }

                // 添加引擎 ID 到全局对象
                // 方便后续流程处理
                context()
                    ->Global()
                    ->Set(
                        context(),
                        v8::String::NewFromUtf8(isolate(), V8_ENGINE_ID).ToLocalChecked(),
                        v8::Number::New(isolate(), engineId)
                    )
                    .Check();

                // TODO: bind native API
                context()
                    ->Global()
                    ->Set(
                        context(),
                        v8::String::NewFromUtf8(isolate(), "register_plugin").ToLocalChecked(),
                        v8::FunctionTemplate::New(isolate(), JSEAPI::register_plugin)
                            ->GetFunction(context())
                            .ToLocalChecked()
                    )
                    .Check();


                return true;
            } catch (const std::exception& e) {
                GetEntry()->getLogger().error("Failed to bind native API: {}", e.what());
            } catch (...) {
                GetEntry()->getLogger().error("Failed to bind native API: Unknown error");
            }
            return false;
        }

    public:
        // [ _bindNativeAPI -> _initESM ] -> loadPluginCode -> initUvLoop
        bool loadPluginCode(const fs::path& file) {
            try {
                _bindNativeAPI(); // 绑定原生API
                _initESM(file);   // 加载ESM

                const char* csource;
                if (!std::filesystem::exists(file)) {
                    GetEntry()->getLogger().error("Failed to load plugin '{}'", file.string());
                    return false;
                }
                std::ifstream t(file);
                std::string   str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
                csource = str.c_str();
                if (csource == nullptr) {
                    GetEntry()->getLogger().error("Failed to load plugin '{}'", file.string());
                    return false;
                }

                // 创建一个包含JavaScript源代码的字符串
                v8::Local<v8::String> source = v8::String::NewFromUtf8(this->isolate(), csource).ToLocalChecked();

                // 编译源代码
                v8::Local<v8::Script> script = v8::Script::Compile(this->context(), source).ToLocalChecked();

                // 执行脚本
                auto __un_used__ = script->Run(this->context());
                return true;
            } catch (const std::exception& e) {
                GetEntry()->getLogger().error("Failed to load plugin code '{}': {}", file.string(), e.what());
            } catch (...) {
                GetEntry()->getLogger().error("Failed to load plugin code '{}'", file.string());
            }
            return false;
        }

        void initUvLoop() {
            auto& sch = GetEntry()->getServer().getScheduler();
            sch.runTaskTimer(
                *GetEntry(),
                [this, event_loop{setup->event_loop()}]() {
                    Locker         locker(isolate());
                    Isolate::Scope isolate_scope{isolate()};
                    uv_run(event_loop, UV_RUN_NOWAIT);
                },
                0,
                2 // 2 tick = 100ms
            );
        }

        void destroy() {
            // 停止事件循环
            GetEntry()->getServer().getScheduler().cancelTask(uvLoopTask->getTaskId());
            uv_stop(setup->event_loop());
            node::Stop(env());
            // 清理 Puerts
            fCppObjectMapper->UnInitialize(isolate());
            // 清理资源
            setup.reset();
        }
    };


public:
    std::unique_ptr<MultiIsolatePlatform> mPlatform;

    std::unordered_map<uint64_t, Engine> mNodeEngines;

    int const _argc    = 2;
    char*     _argv[2] = {
        (char*)"bedrock_server.exe",
        (char*)"",
        // (char*)"--inspect-brk=9229" // 等待调试器连接
    };

public:
    static NodeHelper& getInstance() {
        static NodeHelper instance;
        return instance;
    }

    void initNodeJs() {
        mPlatform = node::MultiIsolatePlatform::Create(std::thread::hardware_concurrency());
        V8::InitializePlatform(mPlatform.get());
        V8::SetFlagsFromString("--no-freeze-flags-after-init"); // 禁止在初始化后修改标志 (防止V8崩溃)
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
        static uint64_t __NextEngineID = 0;
        uint64_t        engineID       = __NextEngineID++;

        fs::path    path       = file;
        std::string pluginName = path.filename().string();

        Engine engine;
        engine.fileName    = pluginName;
        engine.packagePath = file;
        engine.engineId    = engineID;

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

        node::AddEnvironmentCleanupHook(
            engine.setup->isolate(),
            [](void* arg) {
                GetEntry()->getLogger().debug(
                    "Node.js engine destroyed, plugin: {}",
                    std::string(static_cast<const char*>(arg))
                );
            },
            const_cast<void*>(static_cast<const void*>(pluginName.c_str())) // 转换const char*为void*
        );

        // 存储并返回引擎实例
        auto [it, success] = mNodeEngines.emplace(engineID, std::move(engine));
        return &it->second;
    }

    Engine* getEngine(uint64_t engineID) {
        auto it = mNodeEngines.find(engineID);
        if (it == mNodeEngines.end()) {
            return nullptr;
        }
        return &it->second;
    }

    bool destroyEngine(uint64_t engineID) {
        auto it = mNodeEngines.find(engineID);
        if (it == mNodeEngines.end()) {
            return false;
        }

        it->second.destroy();

        mNodeEngines.erase(it);
        return true;
    }

    void shutdownNodeJs() {
        for (auto& [name, engine] : mNodeEngines) {
            engine.destroy();
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