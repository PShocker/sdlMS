#include "sdlms/dynamicsprite.hpp"

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

void DynamicSprite::_draw()
{
    if (std::holds_alternative<Sprite>(_dynamicsprite))
    {
        std::get<Sprite>(_dynamicsprite)._draw();
    }
    else if (std::holds_alternative<AnimatedSprite>(_dynamicsprite))
    {
        std::get<AnimatedSprite>(_dynamicsprite)._draw();
    }
}

[[maybe_unused]] bool DynamicSprite::update(int elapsedTime)
{
    if (std::holds_alternative<AnimatedSprite>(_dynamicsprite))
    {
        return std::get<AnimatedSprite>(_dynamicsprite).update(elapsedTime);
    }
    else if (std::holds_alternative<Sprite>(_dynamicsprite))
    {
        return false;
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

Sprite DynamicSprite::sprite()
{
    if (std::holds_alternative<Sprite>(_dynamicsprite))
    {
        return std::get<Sprite>(_dynamicsprite);
    }
    else if (std::holds_alternative<AnimatedSprite>(_dynamicsprite))
    {
        return std::get<AnimatedSprite>(_dynamicsprite).sprite();
    }
}
