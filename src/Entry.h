#pragma once
#include <endstone/plugin/plugin.h>


class Entry : public endstone::Plugin {
public:
    void onLoad() override;

    void onEnable() override;

    void onDisable() override;
};

extern Entry* GetEntry();