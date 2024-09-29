module;

#include <unordered_map>
#include <string>

export module components:npc;

import :animatedsprite;

export struct Npc
{
    std::unordered_map<std::u16string, AnimatedSprite *> a;
    
    std::u16string index;
};