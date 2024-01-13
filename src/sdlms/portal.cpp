#include "sdlms/portal.hpp"
#include "portal.hpp"

Portal::Portal(std::variant<Sprite, AnimatedSprite> portal, int type, std::u16string url) : _portal(portal),
                                                                                            _type(type),
                                                                                            _url(url)
{
}
void Portal::draw()
{
    if (std::holds_alternative<Sprite>(_portal))
    {
        std::get<Sprite>(_portal).draw();
    }
    else if (std::holds_alternative<AnimatedSprite>(_portal))
    {
        std::get<AnimatedSprite>(_portal).draw();
    }
}