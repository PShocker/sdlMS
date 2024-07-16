module;

#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include <variant>
export module components:backsprite;

import :sprite;
import :animatedsprite;
import resources;

export struct BackSprite
{
    std::variant<Sprite *, AnimatedSprite *> spr;
    // 平铺间隔
    int cx = 0;
    int cy = 0;
    // 是否平铺
    bool htile = false;
    bool vtile = false;

    //平移属性
    int rx = 0;
    int ry = 0;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    bool hspeed = false;
    bool vspeed = false;

    BackSprite()=default;
};