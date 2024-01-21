#ifndef HEADER_UTIL_UI
#define HEADER_UTIL_UI
#include "util/currenton.hpp"
#include "util/map_util.hpp"
#include "sdlms/sprite.hpp"


namespace util
{
    class UIUtil final : public Currenton<UIUtil>
    {
    public:
        std::vector<Sprite> load_minimap();

    private:
        SDL_Renderer *_renderer;
    };
}
#endif