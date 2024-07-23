module;

#include <SDL3/SDL.h>
#include <any>
#include "wz/Property.hpp"

export module components:sprite;

// 这个无法当作组件,因为实体销毁时候会销毁这个组件,使缓存被清掉.
// 如果需要Sprite组件,可以使用StaticSprite
export struct Sprite
{
    SDL_Texture *texture = nullptr;
    int width = 0;
    int height = 0;
    int delay = 0;
    int a0 = 255;
    int a1 = 255;
    SDL_Point origin = {0, 0};
    std::any z = 0;

    Sprite(wz::Node *node, int alpha = 255);
};

export Sprite *load_sprite(wz::Node *node, int alpha = 255);

static std::unordered_map<wz::Node *, Sprite *> sprite_cache;
