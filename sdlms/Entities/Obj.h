#pragma once

#include "wz/Property.hpp"

struct Obj
{
    const static inline unsigned int z_index = 0;
    static void load(wz::Node *node, int id, int layer);
};
