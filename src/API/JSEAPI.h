#pragma once

#include "Engine/Using.h"

namespace jse {

class JSEAPI {
public:
    static Local<Value> registerPlugin(Arguments const& args); // 注册插件

    static Local<Value> getPlugin(Arguments const& args);

    static Local<Value> debug(Arguments const& args);
};

extern ClassDefine<void> JSEAPIClass;

} // namespace jse
