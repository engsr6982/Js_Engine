#pragma once
#include "UsingV8.h"

// 自定义类型转换器
// namespace puerts {
// template <typename T>
// struct ScriptTypeName<std::vector<T*>> {
//     static constexpr auto value() { return internal::Literal("Array"); }
// };

// template <typename T>
// struct ScriptTypeName<std::vector<T*>*> {
//     static constexpr auto value() { return internal::Literal("Array"); }
// };

// // 实现Converter
// namespace v8_impl {
// template <typename T>
// struct Converter<std::vector<T*>> {
//     static v8::Local<v8::Value> toScript(v8::Local<v8::Context> context, const std::vector<T*>& vec) {
//         v8::Isolate*         isolate = context->GetIsolate();
//         v8::Local<v8::Array> jsArray = v8::Array::New(isolate, vec.size());
//         for (size_t i = 0; i < vec.size(); ++i) {
//             jsArray->Set(context, i, Converter<T*>::toScript(context, vec[i])).Check();
//         }
//         return jsArray;
//     }

//     static std::vector<T*> toCpp(v8::Local<v8::Context> context, v8::Local<v8::Value> value) {
//         std::vector<T*> vec;
//         if (value->IsArray()) {
//             v8::Local<v8::Array> jsArray = value.As<v8::Array>();
//             for (uint32_t i = 0; i < jsArray->Length(); ++i) {
//                 vec.push_back(Converter<T*>::toCpp(context, jsArray->Get(context, i).ToLocalChecked()));
//             }
//         }
//         return vec;
//     }
// };

// template <typename T>
// struct Converter<std::vector<T*>*> {
//     static v8::Local<v8::Value> toScript(v8::Local<v8::Context> context, std::vector<T*>* vec) {
//         if (!vec) return v8::Null(context->GetIsolate());
//         return Converter<std::vector<T*>>::toScript(context, *vec);
//     }

//     static std::vector<T*>* toCpp(v8::Local<v8::Context> context, v8::Local<v8::Value> value) {
//         if (value->IsNull() || value->IsUndefined()) return nullptr;
//         auto result = new std::vector<T*>(Converter<std::vector<T*>>::toCpp(context, value));
//         return result;
//     }
// };
// } // namespace v8_impl
// } // namespace puerts
// template <typename T>
// v8::Local<v8::Value> Convert(Isolate* isolate, v8::Local<v8::Context> context, T* value) {
//     return puerts::DataTransfer::FindOrAddCData(isolate, context, puerts::StaticTypeId<T>::get(), value, false);
// }
// template <typename T>
// v8::Local<v8::Value> Convert(Isolate* isolate, v8::Local<v8::Context> context, T& value) {
//     return puerts::DataTransfer::FindOrAddCData(isolate, context, puerts::StaticTypeId<T>::get(), value, false);
// }
