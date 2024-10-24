module;

#include "entt/entt.hpp"

export module entities:aicharacter;

import components;

export
{
    void load_aicharacter(float x, float y, bool sp, entt::entity *ent);
};
