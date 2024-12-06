#pragma once
#include "Node/UsingV8.h"
#include <algorithm>
#include <string>

namespace jse {


inline std::string ReplaceWinPath(const std::string& path, char const& from = '\\', char const& to = '/') {
    std::string fixedPath = path;
    std::replace(fixedPath.begin(), fixedPath.end(), from, to);
    return fixedPath;
}

// V8
inline std::string TryParseString(
    std::string const&     key,
    v8::Local<v8::Object>& pluginInfo,
    v8::Isolate*           isolate,
    v8::Local<v8::Context> context
) {
    auto v = pluginInfo->Get(context, v8::String::NewFromUtf8(isolate, key.c_str()).ToLocalChecked())
                 .ToLocalChecked()
                 ->ToString(context)
                 .ToLocalChecked();
    return *v8::String::Utf8Value(isolate, v);
}

inline std::vector<std::string> TryParseStringArray(
    std::string const&     key,
    v8::Local<v8::Object>& pluginInfo,
    v8::Isolate*           isolate,
    v8::Local<v8::Context> context
) {
    std::vector<std::string> result;

    // 获取属性值
    auto maybeValue = pluginInfo->Get(context, v8::String::NewFromUtf8(isolate, key.c_str()).ToLocalChecked());
    if (maybeValue.IsEmpty()) {
        return result;
    }
    auto value = maybeValue.ToLocalChecked();

    // 检查是否为数组
    if (!value->IsArray()) {
        return result;
    }

    // 转换为数组
    auto     array  = value.As<v8::Array>();
    uint32_t length = array->Length();

    // 遍历数组
    for (uint32_t i = 0; i < length; i++) {
        auto maybeItem = array->Get(context, i);
        if (maybeItem.IsEmpty()) {
            continue;
        }

        auto item = maybeItem.ToLocalChecked();
        if (!item->IsString()) {
            continue;
        }

        // 转换为字符串并添加到结果中
        auto str = item->ToString(context).ToLocalChecked();
        result.push_back(*v8::String::Utf8Value(isolate, str));
    }

    return result;
}


} // namespace jse