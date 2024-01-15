#ifndef HEADER_SDLMS_DYNAMICSPRITE
#define HEADER_SDLMS_DYNAMICSPRITE

#include <variant>

#include "sdlms/animatedsprite.hpp"
#include "sdlms/sprite.hpp"

class DynamicSprite
{
public:
    DynamicSprite(std::variant<Sprite, AnimatedSprite> dynamicsprite);
    void draw();
    void update(int elapsedTime);

public:
    std::variant<Sprite, AnimatedSprite> _dynamicsprite;
};

#endif