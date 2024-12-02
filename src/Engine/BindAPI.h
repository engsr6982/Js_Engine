#pragma once
#include "API/EndStoneAPI.h"
#include "API/LoggerAPI.h"
#include "API/PluginAPI.h"
#include "Using.h"


namespace jse {


inline void BindAPI(ScriptEngine* engine) {
    // static class
    engine->registerNativeClass(EndStoneAPIClass);
    engine->registerNativeClass(LoggerAPIClass);

    // instance class
    engine->registerNativeClass<PluginAPI>(PluginAPIClass);
}


} // namespace jse
