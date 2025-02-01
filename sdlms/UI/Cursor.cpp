#include "Cursor.h"
#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include "Resources/Wz.h"
#include "Components/Components.h"

void Cursor::init()
{
    auto ui_node = Wz::UI->get_root();

    auto node = ui_node->find_from_path(u"Basic.img/Cursor/0/0");

    auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node);
    auto height = canvas->get().height;
    auto width = canvas->get().width;
    auto raw_data = canvas->get_raw_data();

    unsigned char *data = (unsigned char *)SDL_malloc(raw_data.size());
    SDL_memcpy(data, raw_data.data(), raw_data.size());

    auto surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_ARGB4444,
                                         data, width * 2);

    auto cursor = SDL_CreateColorCursor(surface, 0, 0);

    SDL_DestroySurface(surface);
    SDL_free(data);
    
    SDL_SetCursor(cursor);
}
