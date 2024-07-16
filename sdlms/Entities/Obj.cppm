module;

#include "wz/Property.hpp"

export module entities:obj;

export struct Obj
{
    const static inline unsigned int z_index = 0;
    static void load(wz::Node *node, int id, int layer);
};
