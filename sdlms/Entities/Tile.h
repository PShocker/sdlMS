#pragma once

#include "wz/Property.hpp"

struct Tile
{
    const static inline unsigned int z_index = 10000;
    static void load(wz::Node *node, const std::u16string &ts, int layer, int id);
};
