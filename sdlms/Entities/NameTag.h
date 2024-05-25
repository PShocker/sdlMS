#pragma once

#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"

class NameTag : public Entity
{
public:
    NameTag(int width,int height);
    ~NameTag();
};
