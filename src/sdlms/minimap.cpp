#include "sdlms/minimap.hpp"
#include "util/hud_util.hpp"

void MiniMap::draw()
{
    auto nw = _s.find("nw")->second;
    auto ne = _s.find("ne")->second;
    auto sw = _s.find("sw")->second;
    auto se = _s.find("se")->second;

    nw._rect.x = 0;
    nw._rect.x = 0;
    nw._draw();

    ne._rect.x = 130;
    ne._rect.y = 0;
    ne._draw();

    sw._rect.x = 0;
    sw._rect.y = 100;
    sw._draw();

    se._rect.x = 130;
    se._rect.y = 100;
    se._draw();

    auto n = _s.find("n")->second;

    for (size_t i = 1; i < 130; i++)
    {
        n._rect.x = i;
        n._draw();
    }
}