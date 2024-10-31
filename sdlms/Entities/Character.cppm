module;

#include "entt/entt.hpp"

export module entities:character;

import components;

export
{
    void load_character(float x, float y, bool sp, entt::entity ent);
};
