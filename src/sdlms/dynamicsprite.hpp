#ifndef HEADER_SDLMS_DYNAMICSPRITE
#define HEADER_SDLMS_DYNAMICSPRITE

#include <variant>

#include "sdlms/animatedsprite.hpp"
#include "sdlms/sprite.hpp"

class DynamicSprite
{
public:
    DynamicSprite(std::variant<Sprite, AnimatedSprite> dynamicsprite);
    ~DynamicSprite();
    void draw();
    void _draw();
    void update(int elapsedTime);

    SDL_FRect rect();

public:
    std::variant<Sprite, AnimatedSprite> _dynamicsprite;
};

#endif