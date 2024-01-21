#include "sdlms/minimap.hpp"
#include "util/ui_util.hpp"

MiniMap::MiniMap()
{
}

void MiniMap::draw()
{
    auto s = util::UIUtil::current()->load_minimap();
    for (auto &it : s)
    {
        it.draw();
    }
}