module;

#include "entt/entt.hpp"

export module components:ball;

export struct Ball
{
    entt::entity target;
    entt::entity owner;

    Ball() = default;
};