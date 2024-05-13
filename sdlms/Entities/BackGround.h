#pragma once
#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"

class BackGround : public Entity
{
public:
    BackGround(wz::Node *node, int id, World *world);
    ~BackGround();

    enum
    {
        NORMAL,
        HTILED,
        VTILED,
        TILED,
        HMOVEA,
        VMOVEA,
        HMOVEB,
        VMOVEB
    };
};
