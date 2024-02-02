#ifndef HEADER_UTIL_HUD
#define HEADER_UTIL_HUD

#include <vector>
#include <SDL2/SDL.h>

#include "util/currenton.hpp"
#include "util/map_util.hpp"
#include "util/sprite_util.hpp"
#include "util/freetype.hpp"
#include "util/string_util.hpp"

#include "sdlms/sprite.hpp"
#include "sdlms/eventsprite.hpp"

namespace util
{
    class HudUtil final : public Currenton<HudUtil>
    {
    public:
        HudUtil();
        std::vector<Sprite> load_minimap(int mapId);
        std::vector<Sprite> load_statusbar();
        std::vector<EventSprite> load_event_sprite();

    private:
        SDL_Renderer *_renderer;
        SpriteUtil *_sprite_util;
        FreeType *_freetype;
        StringUtil *_string_util;
        MapUtil *_map_util;

    private:
        std::vector<EventSprite> load_event_sprite(wz::Node *node);
    };
}
#endif