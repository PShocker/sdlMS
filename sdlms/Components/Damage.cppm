module;

#include <vector>
#include <SDL3/SDL.h>

export module components:damage;

import :sprite;

export struct Damage
{
    static inline SpriteWarp *red[10];
    static inline SpriteWarp *violet[10];
    static inline SpriteWarp *cri[10];
    static inline SpriteWarp *blue[10];
    static void init();

    struct Info
    {
        int damage;
        float alpha;
        int index;
        char type; // 0:red,1:violet,2:cri,3:blue
    };

    // int alpha = 255;
    std::vector<Info> damage;
    SDL_FPoint *head;
};
