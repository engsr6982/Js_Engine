#pragma once
#include <unordered_map>
#include "Using.h"

namespace jse
{
    struct EngineSelfData
    {
        int mEngineId;
    };
    using EngineSelfDataPtr = std::shared_ptr<EngineSelfData>;

    class EngineManager final
    {
    public:
        EngineManager() = default;
        EngineManager(const EngineManager &) = delete;
        EngineManager &operator=(const EngineManager &) = delete;

        std::unordered_map<int, ScriptEngine *> mEngines;

    public:
        static EngineManager &getInstance();                              // 单例模式
        static EngineSelfDataPtr getEngineSelfData(ScriptEngine *engine); // 获取引擎自身数据
        static void bindAPI(ScriptEngine *engine);                        // 绑定API

        bool hasEngine(int engineId);

        bool destroyEngine(int engineId);

        ScriptEngine *getEngine(int engineId);

        ScriptEngine *createEngine();
    };
}