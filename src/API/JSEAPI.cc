#include "JSEAPI.h"
#include "Node/NodeHelper.h"
#include "Node/UsingV8.h"

namespace jse {


void JSEAPI::register_plugin(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();

    // 检查参数
    if (!info[0]->IsObject()) {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Plugin info must be an object").ToLocalChecked());
        return;
    }

    // 获取当前引擎
    auto engineId = context->Global()
                        ->Get(context, v8::String::NewFromUtf8(isolate, V8_ENGINE_ID).ToLocalChecked())
                        .ToLocalChecked()
                        ->NumberValue(context)
                        .ToChecked();

    auto engine = NodeHelper::getInstance().getEngine(engineId);
    if (!engine) {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to get engine instance").ToLocalChecked());
        return;
    }

    // 保存插件信息(防止GC)
    engine->pluginInfo.Reset(isolate, info[0].As<v8::Object>());
}


} // namespace jse