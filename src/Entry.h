#pragma once
#include "Using.h"
#include <endstone/plugin/plugin.h>


class Entry : public endstone::Plugin {
    std::unique_ptr<MultiIsolatePlatform> mPlatform;

public:
    void onLoad() override;

    void onEnable() override;

    void onDisable() override;
};

extern Entry* GetEntry();