#pragma once

#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"

class NameTag : public Entity
{
public:
    NameTag(int width, int height);
    NameTag(int width, int height, const std::u16string &val);
    int get_width();
    int get_height();

    ~NameTag();
};
