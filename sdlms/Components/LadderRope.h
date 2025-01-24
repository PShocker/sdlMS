#pragma once

#include "wz/Property.hpp"
#include <optional>

struct LadderRope
{
    int x;
    int y1;
    int y2;

    int l;
    int uf;
    int page;

    int t;
    int b;

    LadderRope() = default;
};
