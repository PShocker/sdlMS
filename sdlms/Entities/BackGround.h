#pragma once

#include "wz/Property.hpp"

void load_background(wz::Node *node, int id);
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