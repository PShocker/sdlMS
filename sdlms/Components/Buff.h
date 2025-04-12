#pragma once

#include "entt/entt.hpp"
#include <functional>
#include <any>
#include "Core/Window.h"
#include <unordered_map>
#include "Attack.h"

struct Buff
{
    struct Wrap
    {
        std::optional<std::function<void(entt::entity)>> start;
        std::optional<std::function<void(entt::entity)>> finish;
        std::optional<std::function<void(entt::entity)>> frame;
        std::optional<std::function<void(Attack *, entt::entity)>> before_attack;
        std::optional<std::function<void(Attack *, entt::entity, entt::entity, int)>> after_attack;
        std::optional<std::function<void(Attack *, entt::entity)>> before_hit;
        std::optional<std::function<void(Attack *, entt::entity, entt::entity, int)>> after_hit;
        // 持续时间
        unsigned int duration = 0;
        // 摧毁时间
        unsigned int destory = Window::dt_now + duration;
        // 数据
        std::any data;
    };

    std::unordered_map<int, Wrap> buffs;
    entt::entity owner = entt::null;

    Buff() = default;
};