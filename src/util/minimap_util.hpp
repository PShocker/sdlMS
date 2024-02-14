#ifndef HEADER_UTIL_MINIMAP
#define HEADER_UTIL_MINIMAP

#include <vector>
#include <SDL2/SDL.h>

#include "util/currenton.hpp"
#include "util/map_util.hpp"
#include "util/sprite_util.hpp"
#include "util/freetype_util.hpp"
#include "util/string_util.hpp"

#include "sdlms/sprite.hpp"
#include "sdlms/eventsprite.hpp"

namespace util
{
    class MiniMapUtil final : public Currenton<MiniMapUtil>
    {
    public:
        MiniMapUtil();
        std::vector<Sprite> load_minimap(int mapId);

    private:
        SDL_Renderer *_renderer;
        SpriteUtil *_sprite_util;
        FreeTypeUtil *_freetype_util;
        StringUtil *_string_util;
        MapUtil *_map_util;

    private:
        wz::Node *_ui_node;
    };
}
#endif