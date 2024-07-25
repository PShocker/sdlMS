module;

#include <unordered_map>

export module components:npc;

import :animated;

export struct Npc
{
    std::unordered_map<std::u16string, Animated *> a;
    
    std::u16string index;
};