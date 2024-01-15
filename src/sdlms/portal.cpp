#include "sdlms/portal.hpp"

Portal::Portal(std::variant<Sprite, AnimatedSprite> dynamicsprite, int type, std::u16string url) : DynamicSprite(dynamicsprite),
                                                                                                   _type(type),
                                                                                                   _url(url)
{
}

void Portal::draw()
{
    //只绘制GAME类型的传送门
    if (_type==Type::GAME)
    {
        DynamicSprite::draw();
    }
}
