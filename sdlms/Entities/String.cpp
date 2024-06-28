#include "String.h"

String::String(const std::u16string &s, SDL_Color color, int w, int size)
{
    FreeType::size(size);
    auto str = FreeType::str(s, color, w);
    add_component(str);
}

int String::get_width()
{
    return get_component<Sprite>()->get_width();
}

int String::get_height()
{
    return get_component<Sprite>()->get_height();
}

String::~String()
{
    auto spr = get_component<Sprite>();
    delete spr;
}
