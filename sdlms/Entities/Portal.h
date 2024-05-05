#pragma once

#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"

class Portal : public Entity
{
public:
    Portal(wz::Node *node, std::u16string pn, int pt, int pm, std::u16string tn, World *world);

private:
    std::u16string pn;
    int pt;
    int pm;
    std::u16string tn;
};
