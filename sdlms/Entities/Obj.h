#pragma once

#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"

class Obj : public Entity
{
public:
    Obj(wz::Node *node,int id,int layer, World *world);
    ~Obj();
};
