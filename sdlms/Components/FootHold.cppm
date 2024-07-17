module;
#include "wz/Property.hpp"

export module components:foothold;

export struct FootHold
{
    int page;
    int zmass;
    int prev;
    int next;

    int x1;
    int y1;
    int x2;
    int y2;

    int l;
    int r;
    int t;
    int b;

    static inline std::unordered_map<int, FootHold *> fhs;
    FootHold() = default;
};
