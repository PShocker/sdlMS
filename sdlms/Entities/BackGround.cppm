module;

#include "wz/Property.hpp"

export module entities:background;

export
{
    void load_background(wz::Node * node, int id);
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
