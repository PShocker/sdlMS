#include "Cursor.h"
#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include "Resources/Wz.h"
#include "Components/Components.h"
#include "Commons/Commons.h"

void Cursor::init()
{
    load();
    SDL_HideCursor();
}

void Cursor::load()
{
    auto node = Wz::UI->get_root()->find_from_path(u"Basic.img/Cursor");
    for (auto &[key, val] : node->get_children())
    {
        a[key] = AnimatedSprite(val[0]);
    }
}

void Cursor::run()
{
    Cursor::x = Window::mouse_x;
    Cursor::y = Window::mouse_y;
}

void Cursor::action(const std::u16string &index)
{
    if (index != Cursor::index)
    {
        Cursor::a[Cursor::index].anim_time = 0;
        Cursor::a[Cursor::index].anim_index = 0;
    }
}
