#pragma once

#include "wz/Property.hpp"
#include "Core/World.h"
#include "Components/AnimatedSprite.h"
#include "Components/C_Cursor.h"
#include "Components/Transform.h"

class EB_Cursor
{
public:
    static Entity* BuildEntity(World *world)
    {
        Entity *entity = new Entity();

        C_Cursor *c_Cursor = new C_Cursor(world);
        Transform *c_T = new Transform(0,0,0,true);

        entity->add_component(c_Cursor->aspr_map[c_Cursor->act]);
        entity->add_component(c_Cursor);
        entity->add_component(c_T);

        world->add_component(c_T, 3000000);
        world->add_component(c_Cursor);
        return entity;
    }
};