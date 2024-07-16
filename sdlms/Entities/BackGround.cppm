module;

#include "wz/Property.hpp"

export module entities:background;

export struct BackGround
{
    const static inline unsigned int z_index = -10000;

    static void load(wz::Node *node, int id);
    enum
    {
        NORMAL,
        HTILED,
        VTILED,
        TILED,
        HMOVEA,
        VMOVEA,
        HMOVEB,
        VMOVEB
    };
};
