#include "LoggerAPI.h"
#include "APIHelper.h"
#include "Engine/EngineManager.h"
#include "Engine/EngineSelfData.h"
#include "Entry.h"
#include "fmt/args.h"
#include "fmt/format.h"


namespace jse {

ClassDefine<void> LoggerAPIClass = defineClass("JSE_Logger")
                                       .function("log", &LoggerAPI::log)
                                       .function("info", &LoggerAPI::info)
                                       .function("warn", &LoggerAPI::warn)
                                       .function("error", &LoggerAPI::error)
                                       .function("debug", &LoggerAPI::debug)
                                       .function("color_log", &LoggerAPI::color_log)
                                       .function("format", &LoggerAPI::format)
                                       .build();

Local<Value> LoggerAPIHelper(endstone::Logger::Level level, string const& message) {
    try {
        auto data = ENGINE_SELF_DATA();
        if (data->mPlugin) {
            // data->mPlugin->getLogger().log(level, message); // TODO: fix
            GetEntry()->getLogger().log(level, fmt::format("[{}] {}", data->mPlugin->getName(), message));
            return Boolean::newBoolean(true);
        }
        return Boolean::newBoolean(false);
    }
    Catch;
}
Local<Value> LoggerAPIHelper(endstone::Logger::Level level, Arguments const& args, int start = 0) {
    try {
        std::ostringstream oss;
        for (int i = start; i < args.size(); ++i) ToString(args[i], oss);
        return LoggerAPIHelper(level, oss.str());
    }
    Catch;
}

Local<Value> LoggerAPI::log(Arguments const& args) {
    CheckArgsCount(args, 2);
    CheckArgType(args[0], ValueKind::kNumber);
    // CheckArgType(args[1], ValueKind::kString); // any
    return LoggerAPIHelper(static_cast<endstone::Logger::Level>(args[0].asNumber().toInt64()), args, 1);
}

Local<Value> LoggerAPI::info(Arguments const& args) {
    CheckArgsCount(args, 1);
    // CheckArgType(args[0], ValueKind::kString); // any
    return LoggerAPIHelper(endstone::Logger::Level::Info, args);
}

Local<Value> LoggerAPI::warn(Arguments const& args) {
    CheckArgsCount(args, 1);
    // CheckArgType(args[0], ValueKind::kString); // any
    return LoggerAPIHelper(endstone::Logger::Level::Warning, args);
}

Local<Value> LoggerAPI::error(Arguments const& args) {
    CheckArgsCount(args, 1);
    // CheckArgType(args[0], ValueKind::kString); // any
    return LoggerAPIHelper(endstone::Logger::Level::Error, args);
}

Local<Value> LoggerAPI::debug(Arguments const& args) {
    CheckArgsCount(args, 1);
    // CheckArgType(args[0], ValueKind::kString); // any
    return LoggerAPIHelper(endstone::Logger::Level::Debug, args);
}

Local<Value> LoggerAPI::color_log(Arguments const& args) {
    CheckArgsCount(args, 2);
    CheckArgType(args[0], ValueKind::kString);
    // CheckArgType(args[1], ValueKind::kString); // any

    try {
        std::ostringstream sout;
        const std::string  color = args[0].asString().toString().c_str();
        if (color == "dk_blue") sout << "\x1b[34m";
        else if (color == "dk_green") sout << "\x1b[32m";
        else if (color == "bt_blue") sout << "\x1b[36m";
        else if (color == "dk_red") sout << "\x1b[31m";
        else if (color == "purple") sout << "\x1b[35m";
        else if (color == "dk_yellow") sout << "\x1b[33m";
        else if (color == "grey") sout << "\x1b[37m";
        else if (color == "sky_blue") sout << "\x1b[94m";
        else if (color == "blue") sout << "\x1b[94m";
        else if (color == "green") sout << "\x1b[92m";
        else if (color == "cyan") sout << "\x1b[36m";
        else if (color == "red") sout << "\x1b[91m";
        else if (color == "pink") sout << "\x1b[95m";
        else if (color == "yellow") sout << "\x1b[93m";
        else if (color == "white") sout << "\x1b[97m";
        else {
            PrintScriptError("Invalid color!");
        }
        for (int i = 1; i < args.size(); ++i) ToString(args[i], sout);
        sout << "\x1b[0m";
        return LoggerAPIHelper(endstone::Logger::Level::Info, sout.str());
    }
    Catch;
}

string FormatHelper(Arguments const& args, int offset = 1) {
    try {
        fmt::dynamic_format_arg_store<fmt::format_context> format_args; // 格式化参数

        for (int i = offset; i < args.size(); i++) {
            // 序列化参数
            Local<Value> arg = args[i];
            switch (arg.getKind()) {
            case ValueKind::kString:
                format_args.push_back(arg.asString().toString());
                break;
            case ValueKind::kNumber:
                if (IsFloat(arg)) format_args.push_back(arg.asNumber().toDouble());
                else format_args.push_back(arg.asNumber().toInt32());
                break;
            case ValueKind::kBoolean:
                format_args.push_back(arg.asBoolean().value());
                break;
            default:
                format_args.push_back(ToString(arg));
                break;
            }
        }
        // 格式化参数
        return fmt::vformat(args[0].asString().toString(), format_args);
    } catch (...) {
        return args[0].asString().toString(); // 格式化失败，返回原始字符串
    }
}

Local<Value> LoggerAPI::format(Arguments const& args) {
    CheckArgsCount(args, 1);

    try {
        return String::newString(FormatHelper(args));
    }
    Catch;
}

} // namespace jse