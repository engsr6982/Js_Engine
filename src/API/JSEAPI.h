#pragma once
#include "Node/UsingV8.h"


namespace jse {

class JSEAPI {
public:
    static void register_plugin(const v8::FunctionCallbackInfo<v8::Value>& info);
};

} // namespace jse
