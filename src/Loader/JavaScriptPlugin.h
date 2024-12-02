#pragma once

#include "Engine/EngineManager.h"
#include "endstone/plugin/plugin.h"


namespace jse {

class JavaScriptPlugin : public endstone::Plugin {
public:
    explicit JavaScriptPlugin(int engineId, string const& name, string const& version, string const& description);
    ~JavaScriptPlugin() override;

    void onLoad() override;
    void onEnable() override;
    void onDisable() override;

    [[nodiscard]] const endstone::PluginDescription& getDescription() const override;

public:
    // 存储插件相关的引擎ID
    int engineId_;
    // 存储插件描述信息
    endstone::PluginDescription description_;
};

} // namespace jse
