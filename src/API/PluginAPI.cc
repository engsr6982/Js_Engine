#include "PluginAPI.h"
#include "APIHelper.h"
#include "Engine/EngineSelfData.h"
#include "Engine/Using.h"
#include <cstddef>


namespace jse {

ClassDefine<PluginAPI> PluginAPIClass = defineClass<PluginAPI>("JSE_Plugin")
                                            .constructor(nullptr)
                                            .instanceProperty("api_version", &PluginAPI::api_version)
                                            .instanceProperty("authors", &PluginAPI::authors)
                                            .instanceProperty("commands", &PluginAPI::commands)
                                            .instanceProperty("config", &PluginAPI::config)
                                            .instanceProperty("contributors", &PluginAPI::contributors)
                                            .instanceProperty("default_permission", &PluginAPI::default_permission)
                                            .instanceProperty("depend", &PluginAPI::depend)
                                            .instanceProperty("description", &PluginAPI::description)
                                            .instanceProperty("load", &PluginAPI::load)
                                            .instanceProperty("load_before", &PluginAPI::load_before)
                                            .instanceProperty("name", &PluginAPI::name)
                                            .instanceProperty("permissions", &PluginAPI::permissions)
                                            .instanceProperty("prefix", &PluginAPI::prefix)
                                            .instanceProperty("provides", &PluginAPI::provides)
                                            .instanceProperty("soft_depend", &PluginAPI::soft_depend)
                                            .instanceProperty("version", &PluginAPI::version)
                                            .instanceProperty("website", &PluginAPI::website)

                                            .instanceFunction("register_events", &PluginAPI::register_events)
                                            .instanceFunction("reload_config", &PluginAPI::reload_config)
                                            .instanceFunction("save_config", &PluginAPI::save_config)
                                            .instanceFunction("save_default_config", &PluginAPI::save_default_config)
                                            .instanceFunction("save_resources", &PluginAPI::save_resources)
                                            .build();

ClassDefine<PluginCommandAPI> PluginCommandAPIClass =
    defineClass<PluginCommandAPI>("JSE_PluginCommand").constructor(nullptr).build();

ClassDefine<PluginDescriptionAPI> PluginDescriptionAPIClass =
    defineClass<PluginDescriptionAPI>("JSE_PluginDescription").constructor(nullptr).build();

ClassDefine<PluginLoaderAPI> PluginLoaderAPIClass =
    defineClass<PluginLoaderAPI>("JSE_PluginLoader").constructor(nullptr).build();

ClassDefine<PluginManagerAPI> PluginManagerAPIClass =
    defineClass<PluginManagerAPI>("JSE_PluginManager").constructor(nullptr).build();


// PluginAPI
Local<Value> PluginAPI::api_version() {
    try {
        auto plugin = ENGINE_SELF_DATA()->mPlugin;
        if (plugin) {
            return String::newString(plugin->getDescription().getAPIVersion());
        }
        return Local<Value>();
    }
    Catch;
}

Local<Value> PluginAPI::authors() {
    try {
        auto plugin = ENGINE_SELF_DATA()->mPlugin;
        if (plugin) {
            auto authors = plugin->getDescription().getAuthors();
            auto arr     = Array::newArray(authors.size());
            for (size_t i = 0; i < authors.size(); i++) {
                arr.add(String::newString(authors[i]));
            }
            return arr;
        }
        return Local<Value>();
    }
    Catch;
}

Local<Value> PluginAPI::commands() { return Local<Value>(); }

Local<Value> PluginAPI::config() { return Local<Value>(); }

Local<Value> PluginAPI::contributors() { return Local<Value>(); }

Local<Value> PluginAPI::default_permission() { return Local<Value>(); }

Local<Value> PluginAPI::depend() { return Local<Value>(); }

Local<Value> PluginAPI::description() { return Local<Value>(); }

Local<Value> PluginAPI::load() { return Local<Value>(); }

Local<Value> PluginAPI::load_before() { return Local<Value>(); }

Local<Value> PluginAPI::name() { return Local<Value>(); }

Local<Value> PluginAPI::permissions() { return Local<Value>(); }

Local<Value> PluginAPI::prefix() { return Local<Value>(); }

Local<Value> PluginAPI::provides() { return Local<Value>(); }

Local<Value> PluginAPI::soft_depend() { return Local<Value>(); }

Local<Value> PluginAPI::version() { return Local<Value>(); }

Local<Value> PluginAPI::website() { return Local<Value>(); }

Local<Value> PluginAPI::register_events(Arguments const& args) { return Local<Value>(); }

Local<Value> PluginAPI::reload_config(Arguments const& args) { return Local<Value>(); }

Local<Value> PluginAPI::save_config(Arguments const& args) { return Local<Value>(); }

Local<Value> PluginAPI::save_default_config(Arguments const& args) { return Local<Value>(); }

Local<Value> PluginAPI::save_resources(Arguments const& args) { return Local<Value>(); }


} // namespace jse