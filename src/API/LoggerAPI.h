#pragma once
#include "Engine/Using.h"

namespace jse {
class LoggerAPI {
public:
    static Local<Value> log(Arguments const& args);
    static Local<Value> info(Arguments const& args);
    static Local<Value> warn(Arguments const& args);
    static Local<Value> error(Arguments const& args);
    static Local<Value> debug(Arguments const& args);

    static Local<Value> color_log(Arguments const& args);
    static Local<Value> format(Arguments const& args);
};

extern ClassDefine<void> LoggerAPIClass;

} // namespace jse