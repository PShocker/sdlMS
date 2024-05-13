#pragma once
#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"

class LadderRope : public Entity
{
public:
    LadderRope(wz::Node *node, int id, World *world);
    ~LadderRope();

public:
    // lr的附加属性
    int id;
    int l;
    int uf;
    int page;
};