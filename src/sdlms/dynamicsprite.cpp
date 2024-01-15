#include "sdlms/dynamicsprite.hpp"

DynamicSprite::DynamicSprite(std::variant<Sprite, AnimatedSprite> dynamicsprite) : _dynamicsprite(dynamicsprite)
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
