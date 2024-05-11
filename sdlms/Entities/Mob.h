#pragma once

#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"
#include "Components/AnimatedSprite.h"

class Mob : public Entity
{
public:
    Mob(wz::Node *node, World *world);
    Mob(World *world);

    void switch_act(const std::u16string &a);

private:
    std::unordered_map<std::u16string, AnimatedSprite *> aspr_map;
    std::u16string act = u"jump";
    Sprite *name = nullptr;
};