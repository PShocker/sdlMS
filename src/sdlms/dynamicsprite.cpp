#include "sdlms/dynamicsprite.hpp"
#include "dynamicsprite.hpp"

DynamicSprite::DynamicSprite(std::variant<Sprite, AnimatedSprite> dynamicsprite) : _dynamicsprite(dynamicsprite)
{
}

DynamicSprite::~DynamicSprite()
{
}

void DynamicSprite::draw()
{
    if (std::holds_alternative<Sprite>(_dynamicsprite))
    {
        std::get<Sprite>(_dynamicsprite).draw();
    }
    else if (std::holds_alternative<AnimatedSprite>(_dynamicsprite))
    {
        std::get<AnimatedSprite>(_dynamicsprite).draw();
    }
}

void DynamicSprite::update(int elapsedTime)
{
    if (std::holds_alternative<AnimatedSprite>(_dynamicsprite))
    {
        std::get<AnimatedSprite>(_dynamicsprite).update(elapsedTime);
    }
}

SDL_FRect DynamicSprite::rect()
{
    if (std::holds_alternative<Sprite>(_dynamicsprite))
    {
        return std::get<Sprite>(_dynamicsprite).rect();
    }
    else if (std::holds_alternative<AnimatedSprite>(_dynamicsprite))
    {
        return std::get<AnimatedSprite>(_dynamicsprite).rect();
    }
    return SDL_FRect{};
}
