module;

#include <vector>

export module components:damage;

import :sprite;

export struct Damage
{
    static inline SpriteWarp *sprs[10];
    static void init();

    unsigned char alpha;
    std::vector<int> damage;
};
