#ifndef HEADER_UTIL_HUD
#define HEADER_UTIL_HUD

#include <vector>
#include <SDL2/SDL.h>

#include "util/currenton.hpp"
#include "util/map_util.hpp"
#include "sdlms/sprite.hpp"

namespace util
{
    class HudUtil final : public Currenton<HudUtil>
    {
    public:
        HudUtil();
        std::vector<Sprite> load_minimap(int mapId);
        std::vector<Sprite> load_statusbar();

    private:
        SDL_Renderer *_renderer;

    private:
        Sprite load_sprite(wz::Property<wz::WzCanvas> *canvas);
    };
}
#endif