#pragma once

#include "Entity.h"
#include "wz/Property.hpp"

class Tile : public Entity
{
public:
    Tile(wz::Node *node, std::u16string ts);
};
