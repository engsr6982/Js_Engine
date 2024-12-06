#pragma once
#include "endstone/plugin/plugin.h"
#include <cstdint>


namespace jse {

class JavaScriptPlugin : public endstone::Plugin {
public:
    explicit JavaScriptPlugin(
        uint64_t                                                engineId,
        std::string                                             name,
        std::string                                             version,
        const std::optional<std::string>&                       description,
        std::optional<endstone::PluginLoadOrder>                load,
        const std::optional<std::vector<std::string>>&          authors,
        const std::optional<std::vector<std::string>>&          contributors,
        const std::optional<std::string>&                       website,
        const std::optional<std::string>&                       prefix,
        const std::optional<std::vector<std::string>>&          provides,
        const std::optional<std::vector<std::string>>&          depend,
        const std::optional<std::vector<std::string>>&          soft_depend,
        const std::optional<std::vector<std::string>>&          load_before,
        std::optional<endstone::PermissionDefault>              default_permission,
        const std::optional<std::vector<endstone::Command>>&    commands,
        const std::optional<std::vector<endstone::Permission>>& permissions
    );
    ~JavaScriptPlugin() override;

    void onLoad() override;
    void onEnable() override;
    void onDisable() override;

    [[nodiscard]] const endstone::PluginDescription& getDescription() const override;

public:
    endstone::PluginDescription description_;
    uint64_t                    engineId_;
};

} // namespace jse
