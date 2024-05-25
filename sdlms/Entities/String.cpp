#include "String.h"

String::String(const std::u16string &s, int font_size)
{
    auto str = FreeType::str(s);
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
