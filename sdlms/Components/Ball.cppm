module;

#include "entt/entt.hpp"

export module components:ball;

export struct Ball
{
    entt::entity *target = nullptr;
    entt::entity *owner = nullptr;
    float rotation = 0;

    Ball() = default;
};