#pragma once
#include "endstone/plugin/plugin.h"


namespace jse {

class JavaScriptPlugin : public endstone::Plugin {
public:
    explicit JavaScriptPlugin(std::string const& name, std::string const& version, std::string const& description);
    ~JavaScriptPlugin() override;

    void onLoad() override;
    void onEnable() override;
    void onDisable() override;

    [[nodiscard]] const endstone::PluginDescription& getDescription() const override;

public:
    endstone::PluginDescription description_;
};

} // namespace jse
