#pragma once

#include "wz/Property.hpp"
#include "entt/entt.hpp"

entt::entity load_summon(wz::Node *node, const u16string &id, entt::entity owner);
