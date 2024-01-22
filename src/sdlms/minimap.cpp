#include "sdlms/minimap.hpp"
#include "util/hud_util.hpp"

MiniMap::MiniMap()
{
}

void MiniMap::draw()
{
    auto s = util::HudUtil::current()->load_minimap();
    for (auto &it : s)
    {
        it.draw();
    }
}