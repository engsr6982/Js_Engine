#pragma once

#include "Engine/EngineManager.h"
#include "endstone/detail/server.h"
#include "endstone/plugin/plugin_loader.h"


namespace jse {

class JavaScriptPluginLoader : public endstone::PluginLoader {
public:
    explicit JavaScriptPluginLoader(endstone::Server& server);
    ~JavaScriptPluginLoader() override = default;

    // 加载指定目录下的所有JS插件
    [[nodiscard]] std::vector<endstone::Plugin*> loadPlugins(const std::string& directory) override;

    // 启用插件
    void enablePlugin(endstone::Plugin& plugin) const override;

    // 禁用插件
    void disablePlugin(endstone::Plugin& plugin) const override;

private:
    // 加载单个JS插件
    endstone::Plugin* loadPlugin(const std::filesystem::path& file);
};

} // namespace jse