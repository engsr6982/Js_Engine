#pragma once

#include "Engine/EngineManager.h"
#include "Engine/Using.h"
#include "endstone/command/command.h"
#include "endstone/command/plugin_command.h"
#include "endstone/detail/server.h"
#include "endstone/plugin/plugin.h"

namespace jse {


class PluginAPI : public ScriptClass {
public:
    explicit PluginAPI() : ScriptClass(ScriptClass::ConstructFromCpp<PluginAPI>{}) {}

    static Local<Object> newPluginAPI() { return (new PluginAPI())->getScriptObject(); }

public:
    // properties
    Local<Value> api_version();
    Local<Value> authors();
    Local<Value> commands();
    Local<Value> config();
    Local<Value> contributors();
    Local<Value> default_permission();
    Local<Value> depend();
    Local<Value> description();
    Local<Value> load();
    Local<Value> load_before();
    Local<Value> name();
    Local<Value> permissions();
    Local<Value> prefix();
    Local<Value> provides();
    Local<Value> soft_depend();
    Local<Value> version();
    Local<Value> website();

    // methods
    Local<Value> register_events(Arguments const& args);
    Local<Value> reload_config(Arguments const& args);
    Local<Value> save_config(Arguments const& args);
    Local<Value> save_default_config(Arguments const& args);
    Local<Value> save_resources(Arguments const& args);
};

class PluginCommandAPI : public ScriptClass {
public:
    // properties
    Local<Value> executor();
    Local<Value> plugin();
};

class PluginDescriptionAPI : public ScriptClass {
public:
    Local<Value> api_version();
    Local<Value> authors();
    Local<Value> commands();
    Local<Value> contributors();
    Local<Value> default_permission();
    Local<Value> depend();
    Local<Value> description();
    Local<Value> full_name();
    Local<Value> load();
    Local<Value> load_before();
    Local<Value> name();
    Local<Value> permissions();
    Local<Value> prefix();
    Local<Value> provides();
    Local<Value> soft_depend();
    Local<Value> version();
    Local<Value> website();
};

class PluginLoaderAPI : public ScriptClass {
public:
    // properties
    Local<Value> server();

    // methods
    Local<Value> load_plugins(Arguments const& args);
    Local<Value> enable_plugin(Arguments const& args);
    Local<Value> disable_plugin(Arguments const& args);
};

class PluginManagerAPI : public ScriptClass {
public:
    // properties
    Local<Value> plugins();
    Local<Value> permissions();

    // methods
    Local<Value> call_event(Arguments const& args);
    Local<Value> clear_plugins(Arguments const& args);
    Local<Value> disable_plugin(Arguments const& args);
    Local<Value> disable_plugins(Arguments const& args);
    Local<Value> enable_plugin(Arguments const& args);
    Local<Value> enable_plugins(Arguments const& args);
    Local<Value> get_default_perm_subscriptions(Arguments const& args);
    Local<Value> get_default_permissions(Arguments const& args);
    Local<Value> get_permission(Arguments const& args);
    Local<Value> get_permission_subscriptions(Arguments const& args);
    Local<Value> get_plugin(Arguments const& args);
    Local<Value> is_plugin_enabled(Arguments const& args);
    // Local<Value> is_plugin_enabled(Arguments const& args); // overloaded
    Local<Value> load_plugins(Arguments const& args);
    Local<Value> recalculate_permission_defaults(Arguments const& args);
    Local<Value> register_event(Arguments const& args);
    Local<Value> remove_permission(Arguments const& args);
    // Local<Value> remove_permission(Arguments const& args); // overloaded
    Local<Value> subscribe_to_default_perms(Arguments const& args);
    Local<Value> subscribe_to_permission(Arguments const& args);
    Local<Value> unsubscribe_from_default_perms(Arguments const& args);
    Local<Value> unsubscribe_from_permission(Arguments const& args);
};

extern ClassDefine<PluginAPI>            PluginAPIClass;
extern ClassDefine<PluginCommandAPI>     PluginCommandAPIClass;
extern ClassDefine<PluginDescriptionAPI> PluginDescriptionAPIClass;
extern ClassDefine<PluginLoaderAPI>      PluginLoaderAPIClass;
extern ClassDefine<PluginManagerAPI>     PluginManagerAPIClass;


} // namespace jse