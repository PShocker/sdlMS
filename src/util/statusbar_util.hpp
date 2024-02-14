#ifndef HEADER_UTIL_STATUSBAR
#define HEADER_UTIL_STATUSBAR

#include <vector>
#include <SDL2/SDL.h>

#include "template/currenton.hpp"
#include "util/map_util.hpp"
#include "util/sprite_util.hpp"
#include "util/freetype_util.hpp"
#include "util/string_util.hpp"

#include "sdlms/sprite.hpp"
#include "sdlms/eventsprite.hpp"

namespace util
{
    class StatusBarUtil final : public Currenton<StatusBarUtil>
    {
    public:
        StatusBarUtil();
        std::vector<Sprite> load_statusbar();
        std::vector<EventSprite*> load_event_sprite();

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