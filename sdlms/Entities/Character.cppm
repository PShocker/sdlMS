module;

#include "entt/entt.hpp"

export module entities:character;

import components;

export
{
    entt::entity *load_character(float x, float y, bool sp,  entt::entity *ent = nullptr);
};
