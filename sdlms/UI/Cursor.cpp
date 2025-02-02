#include "Cursor.h"
#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include "Resources/Wz.h"
#include "Components/Components.h"

void Cursor::init()
{
    load(u"0");
    load(u"12");

    SDL_SetCursor(cursor[u"0"][0]);
}

void Cursor::load(std::u16string path)
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

        cursor[path].push_back(SDL_CreateColorCursor(surface, 0, 0));

        SDL_DestroySurface(surface);
        SDL_free(data);
    }
}
