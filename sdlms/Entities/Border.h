#pragma once
#include "Entity.h"
#include "wz/Property.hpp"

class Border : public Entity
{
public:
    Border(wz::Node *node, World *world);

private:
    float left = 0;
    float right = 0;
    float top = 0;
    float bottom = 0;
};
