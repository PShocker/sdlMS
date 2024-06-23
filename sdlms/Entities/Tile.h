#pragma once

#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"

class Tile : public Entity
{
public:
    Tile(wz::Node *node, const std::u16string &ts, int layer, int id, World *world);
    ~Tile();
};
