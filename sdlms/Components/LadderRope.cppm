module;
#include "wz/Property.hpp"
#include <optional>

export module components:ladderrope;

export struct LadderRope
{
    int x;
    int y1;
    int y2;

    int l;
    int uf;
    int page;

    int t;
    int b;

    static inline std::unordered_map<int, LadderRope *> lrs;

    LadderRope() = default;
};
