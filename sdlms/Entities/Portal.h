#pragma once

#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"
#include "Components/AnimatedSprite.h"


class Portal : public Entity
{
public:
    Portal(wz::Node *node, World *world);

    std::unordered_map<std::u16string, AnimatedSprite *> aspr_map;
    int tm=0;
};
