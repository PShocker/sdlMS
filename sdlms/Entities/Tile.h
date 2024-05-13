#pragma once

#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"

class Tile : public Entity
{
public:
    Tile(wz::Node *node, std::u16string ts,int layer, World *world);
    ~Tile();
};
