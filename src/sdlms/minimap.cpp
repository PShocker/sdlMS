#include "sdlms/minimap.hpp"
#include "util/hud_util.hpp"

void MiniMap::draw()
{
    auto nw = _vec.find("nw")->second;
    auto ne = _vec.find("ne")->second;
    auto sw = _vec.find("sw")->second;
    auto se = _vec.find("se")->second;

    nw._rect.x = 0;
    nw._rect.x = 0;
    nw._draw();

    ne._rect.x = 200;
    ne._rect.y = 0;
    ne._draw();

    sw._rect.x = 0;
    sw._rect.y = 140;
    sw._draw();

    se._rect.x = 200;
    se._rect.y = 140;
    se._draw();

    auto n = _vec.find("n")->second;

    for (size_t x = 6; x <= 200; x++)
    {
        n._rect.x = x;
        n._draw();
    }

    auto s = _vec.find("s")->second;

    s._rect.x = 6;
    s._rect.y = 140;

    for (size_t x = 6; x <= 200; x++)
    {
        s._rect.x = x;
        s._draw();
    }

    auto w = _vec.find("w")->second;

    w._rect.x = 0;
    w._rect.y = 0;

    for (size_t y = 72; y < 140; y++)
    {
        w._rect.y = y;
        w._draw();
    }

    auto e = _vec.find("e")->second;

    e._rect.x = 200;
    e._rect.y = 0;

    for (size_t y = 72; y < 140; y++)
    {
        e._rect.y = y;
        e._draw();
    }

}