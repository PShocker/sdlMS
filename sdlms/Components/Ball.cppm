module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>

export module components:ball;

import core;

export struct Ball
{
    entt::entity target = entt::null;
    entt::entity owner = entt::null;

    std::optional<SDL_FPoint> p = std::nullopt;

    // 摧毁时间
    unsigned int destory = Window::dt_now + 800;

    Ball() = default;
};