#pragma once

#include "AnimatedSprite.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>
#include "Core/Window.h"

struct Ball
{
    entt::entity target = entt::null;
    entt::entity owner = entt::null;

    std::optional<SDL_FPoint> target_point = std::nullopt;

    std::optional<SDL_FPoint> point = std::nullopt;

    AnimatedSprite::Wrap *hit = nullptr;

    // 摧毁时间
    unsigned int destory = Window::dt_now;

    std::optional<int> rotate = std::nullopt;

    // 是否需要改变轨迹跟踪怪物
    bool track = true;
    std::optional<std::unordered_set<entt::entity>> track_hit = std::nullopt;

    Ball() = default;
};