module;

#include <string>
#include <unordered_map>

export module ui:cursor;

import components;

export struct Cursor
{
    static inline std::unordered_map<std::u16string, Animated *> cursor;

    static void init();
};
