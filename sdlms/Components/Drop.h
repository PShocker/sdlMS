#pragma once

#include <array>
#include <string>
#include "entt/entt.hpp"
#include "AnimatedSprite.h"
#include "Core/Core.h"

struct Drop
{
    std::u16string id;
    uint64_t nums;
    int rotate;
    bool land = false;
    float land_y;
    bool attract = false;
    entt::entity picker = entt::null;

    std::variant<Sprite, AnimatedSprite> spr;

    // 摧毁时间
    unsigned int destory = Window::dt_now + 20000;

    enum MesoIcon
    {
        BRONZE,
        GOLD,
        BUNDLE,
        BAG,
        NUM_ICONS
    };

    Drop() = default;

    static inline std::array<AnimatedSprite::Wrap *, NUM_ICONS> mesoicons;
    static void init();
};
