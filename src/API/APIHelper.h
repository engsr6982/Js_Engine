#pragma once
#include "Engine/Using.h"
#include "Entry.h"
#include "fmt/format.h"
#include <iostream>


namespace jse {
template <typename T>
bool FindVector(std::vector<T> const& vec, T const& value);

bool IsFloat(Local<Value> const& num);

string ToString(ValueKind const& kind);
string ToString(Local<Value> const& value);
void   ToString(Local<Value> const& value, std::ostringstream& oss);
void   ToString(Local<Array> const& value, std::ostringstream& oss);
void   ToString(Local<Object> const& value, std::ostringstream& oss);

template <typename T>
bool IsInstanceOf(Local<Value> const& value);

void PrintException(string const& msg, string const& func, string const& plugin, string const& api);
void PrintException(script::Exception const& e, string const& func, string const& plugin, string const& api);

#define PrintScriptError(msg_or_exception)                                                                             \
    PrintException(msg_or_exception, __func__, ENGINE_SELF_DATA()->mJSE_Plugin.getFileName(), __FUNCTION__);

// 参数异常
#define PrintWrongArgType()   PrintScriptError("Wrong argument type")
#define PrintArgsTooFew()     PrintScriptError("Too few arguments")
#define PrintWrongArgsCount() PrintScriptError("Wrong arguments count")

// 检查参数数量
#define CheckArgsCount(args, count)                                                                                    \
    if (args.size() < count) {                                                                                         \
        PrintArgsTooFew();                                                                                             \
        return Local<Value>();                                                                                         \
    }

// 检查参数类型
#define CheckArgType(arg, type)                                                                                        \
    if (arg.getKind() != type) {                                                                                       \
        PrintWrongArgType();                                                                                           \
        return Local<Value>();                                                                                         \
    }

// 异常捕获
#define Catch                                                                                                          \
    catch (script::Exception const& e) {                                                                               \
        PrintScriptError(e) return Local<Value>();                                                                     \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        PrintScriptError("Unknown error");                                                                             \
        return Local<Value>();                                                                                         \
    }

#define CatchNotReturn                                                                                                 \
    catch (script::Exception const& e) {                                                                               \
        PrintScriptError(e)                                                                                            \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        PrintScriptError("Unknown error");                                                                             \
    }

} // namespace jse