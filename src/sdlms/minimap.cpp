#include "sdlms/minimap.hpp"
#include "util/hud_util.hpp"

void MiniMap::draw()
{

    for (auto &it : _s)
    {
        it._draw();
    }
}