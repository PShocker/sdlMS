#pragma once
#include "Entity.h"
#include "wz/Property.hpp"

class BackGround : public Entity
{
    BackGround(wz::Node *node, int id, World *world);
};
