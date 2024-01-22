#ifndef HEADER_UTIL_UI
#define HEADER_UTIL_UI
#include <map>
#include "util/currenton.hpp"
#include "util/map_util.hpp"
#include "sdlms/sprite.hpp"

namespace util
{
    class HudUtil final : public Currenton<HudUtil>
    {
    public:
        std::map<std::string, Sprite> load_minimap();

    private:
        SDL_Renderer *_renderer;
    };
}
#endif