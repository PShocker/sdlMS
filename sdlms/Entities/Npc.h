#pragma once

#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"

class Npc : public Entity
{
public:
    Npc(wz::Node *node, std::u16string ts,int layer, World *world);
};
