module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

export module components:ball;

export struct Ball
{
    entt::entity target = entt::null;
    entt::entity owner = entt::null;

    SDL_FPoint p;

    Ball() = default;
};