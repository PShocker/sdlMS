#include "sdlms/portal.hpp"
#include "portal.hpp"

Portal::Portal(std::variant<Sprite, AnimatedSprite> dynamicsprite, int type, std::u16string url) : DynamicSprite(dynamicsprite),
                                                                                                   _type(type),
                                                                                                   _url(url)
{
}