#pragma once

#include "entt/entt.hpp"
#include <functional>
#include "Core/Window.h"
#include <unordered_map>

struct Buff
{
    struct Info
    {
        std::optional<std::function<void(entt::entity)>> start;
        std::optional<std::function<void(entt::entity)>> finish;
        std::optional<std::function<void(entt::entity)>> before_attack;
        std::optional<std::function<void(entt::entity)>> after_attack;
        std::optional<std::function<void(entt::entity)>> before_hit;
        std::optional<std::function<void(entt::entity)>> after_hit;
        // 持续时间
        unsigned int duration = 30000;
        // 摧毁时间
        unsigned int destory = Window::dt_now + duration;
    };

    std::unordered_map<std::u16string, Info> buffs;
    entt::entity owner = entt::null;

    Buff() = default;
};