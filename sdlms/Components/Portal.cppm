module;
#include <string>
#include <unordered_map>
export module components:portal;

import :transform;

export struct Portal
{
    int tm = 0;
    int pt = 0;
    std::u16string pn;
    std::u16string tn;

    Transform *tr = nullptr;

    // key:tn , value:portal
    static inline std::unordered_map<std::u16string, Portal *> pors;

    Portal() = default;
};
