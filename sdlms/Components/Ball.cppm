module;

#include "entt/entt.hpp"

export module components:ball;

export struct Ball
{
    entt::entity target = entt::null;
    entt::entity owner = entt::null;

    Ball() = default;
};