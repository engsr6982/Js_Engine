#pragma once
#include "Using.h"
#include <unordered_map>


#include "endstone/plugin/plugin.h"

namespace jse {

struct JSE_Plugin {
    string                 mFileName;
    script::Global<Object> mPlugin;

    string getName() const {
        if (mPlugin.isEmpty()) {
            return mFileName;
        }
        if (!mPlugin.get().has("name")) {
            return mFileName;
        }
        return mPlugin.get().get("name").asString().toString();
    }
    string getVersion() const {
        if (mPlugin.isEmpty()) {
            return "0.0.0";
        }
        if (!mPlugin.get().has("version")) {
            return "0.0.0";
        }
        return mPlugin.get().get("version").asString().toString();
    }
    string getDescription() const {
        if (mPlugin.isEmpty()) {
            return "";
        }
        if (!mPlugin.get().has("description")) {
            return "";
        }
        return mPlugin.get().get("description").asString().toString();
    }
    string getFileName() const { return mFileName; }

    bool callOnLoad() {
        if (mPlugin.isEmpty()) return false;
        if (!mPlugin.get().has("onLoad")) return false;
        auto func = mPlugin.get().get("onLoad");
        if (!func.isFunction()) return false;
        func.asFunction().call();
        return true;
    }
    bool callOnEnable() {
        if (mPlugin.isEmpty()) return false;
        if (!mPlugin.get().has("onEnable")) return false;
        auto func = mPlugin.get().get("onEnable");
        if (!func.isFunction()) return false;
        func.asFunction().call();
        return true;
    }
    bool callOnDisable() {
        if (mPlugin.isEmpty()) return false;
        if (!mPlugin.get().has("onDisable")) return false;
        auto func = mPlugin.get().get("onDisable");
        if (!func.isFunction()) return false;
        func.asFunction().call();
        return true;
    }

    operator script::Global<Object>() { return mPlugin; }
    void operator=(script::Global<Object> plugin) { mPlugin = plugin; }

    JSE_Plugin()                       = default;
    JSE_Plugin(JSE_Plugin&)            = delete;
    JSE_Plugin& operator=(JSE_Plugin&) = delete;
};

struct EngineSelfData {
    int               mEngineId{-1};
    endstone::Plugin* mPlugin{nullptr};
    JSE_Plugin        mJSE_Plugin; // JSE_EndStone.registerPlugin
};
using EngineSelfDataPtr = std::shared_ptr<EngineSelfData>;


#define GET_ENGINE_SELF_DATA(enginePtr) enginePtr->getData<EngineSelfData>()
#define ENGINE_SELF_DATA()              EngineScope ::currentEngine()->getData<EngineSelfData>()

} // namespace jse
