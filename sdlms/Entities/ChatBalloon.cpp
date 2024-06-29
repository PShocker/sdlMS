#include "ChatBalloon.h"
#include "Resource/Wz.h"

ChatBalloon::ChatBalloon(int width, int height, const std::u16string &val)
{
    spr = new Sprite(World::get_world()->get_resource<Wz>().UI->get_root()->find_from_path(u"ChatBalloon.img/" + val),
                     width, height, Sprite::ChatBallon);
    add_component(spr);
}

int ChatBalloon::get_width()
{
    return get_component<Sprite>()->get_width();
}

int ChatBalloon::get_height()
{
    return get_component<Sprite>()->get_height();
}

ChatBalloon::~ChatBalloon()
{
    delete spr;
}
