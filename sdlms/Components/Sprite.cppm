module;

#include <SDL3/SDL.h>
#include <any>
#include "wz/Property.hpp"
#include <optional>

export module components:sprite;

export struct SpriteWarp
{
    SDL_Texture *texture = nullptr;
    int width = 0;
    int height = 0;
    int delay = 0;
    int a0 = 255;
    int a1 = 255;
    SDL_Point origin = {0, 0};
    std::any z = 0;

    std::optional<SDL_Point> lt;
    std::optional<SDL_Point> rb;
    std::optional<SDL_FPoint> head;
    std::optional<SDL_FRect> rect;

    static SpriteWarp *load(wz::Node *node, int alpha = 255);

    SpriteWarp(wz::Node *node, int alpha = 255);
};


export struct Sprite
{
    SpriteWarp *spr = nullptr;
    Sprite(wz::Node *node, int alpha = 255);
    Sprite() = default;
};

