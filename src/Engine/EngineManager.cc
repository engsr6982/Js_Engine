#include "EngineManager.h"
#include "BindAPI.h"
#include "EngineSelfData.h"

namespace jse {
EngineManager& EngineManager::getInstance() {
    static EngineManager instance;
    return instance;
}

bool EngineManager::hasEngine(int engineId) { return this->mEngines.contains(engineId); }

bool EngineManager::destroyEngine(int engineId) {
    if (!this->hasEngine(engineId)) {
        return false;
    }

    auto engine = this->mEngines[engineId];
    engine->destroy(); // 销毁引擎

    this->mEngines.erase(engineId);
    return true;
}

ScriptEngine* EngineManager::getEngine(int engineId) {
    if (!this->hasEngine(engineId)) {
        return nullptr;
    }

    return this->mEngines[engineId];
}

ScriptEngine* EngineManager::createEngine() {
    static int mNextEngineId = 0;

    int const engineId = mNextEngineId++;

    auto engine              = new ScriptEngineImpl();
    this->mEngines[engineId] = engine;

    EngineScope scope(engine);                                   // 进入引擎作用域
    BindAPI(engine);                                             // 绑定API
    engine->setData(std::make_shared<EngineSelfData>(engineId)); // 设置引擎自身数据

    return engine;
}
} // namespace jse
