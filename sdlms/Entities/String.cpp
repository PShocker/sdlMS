#include "String.h"

String::String(const std::u16string &s, SDL_Color color, int size)
{
    FreeType::size(size);
    auto str = FreeType::str(s, color);
    add_component(str);
}

String::~String()
{
    if (get_component<Sprite>() != nullptr)
    {
        auto spr = get_component<Sprite>();
        delete spr;
    }
}
