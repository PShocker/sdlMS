module;

#include "entt/entt.hpp"

export module entities:character;

import components;

export
{
    entt::entity* load_character();
};
