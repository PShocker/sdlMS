#include "Cursor.h"
#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include "Resources/Wz.h"
#include "Components/Components.h"

void Cursor::init()
{
    load(u"0");
    load(u"1");
    load(u"12");

    Cursor::type = u"0";
}

void Cursor::load(const std::u16string &path)
{
    auto ui_node = Wz::UI->get_root();

    auto node = ui_node->find_from_path(u"Basic.img/Cursor/" + path);
    for (auto &[key, val] : node->get_children())
    {
        auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(val[0]);
        auto height = canvas->get().height;
        auto width = canvas->get().width;
        auto raw_data = canvas->get_raw_data();

        unsigned char *data = (unsigned char *)SDL_malloc(raw_data.size());
        SDL_memcpy(data, raw_data.data(), raw_data.size());

        auto surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_ARGB4444,
                                             data, width * 2);

        int delay = 0;
        if (val[0]->get_child(u"delay"))
        {
            delay = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"delay"))->get();
        }

        cursor[path].push_back({SDL_CreateColorCursor(surface, 0, 0), delay});

        SDL_DestroySurface(surface);
        SDL_free(data);
    }
}

void Cursor::run()
{
    if (cursor[type].size() > 1)
    {
        auto delay = cursor[type][index].delay;
        time += Window::delta_time;
        if (time >= delay)
        {
            time = 0;
            index++;
            if (index >= cursor[type].size())
            {
                index = 0;
            }
            SDL_SetCursor(cursor[type][index].cursor);
        }
    }
}

void Cursor::action(const std::u16string &t)
{
    if (t != Cursor::type)
    {
        time = 0;
        index = 0;
        SDL_SetCursor(cursor[Cursor::type][index].cursor);
    }
}
