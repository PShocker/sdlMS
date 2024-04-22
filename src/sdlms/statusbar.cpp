#include "sdlms/statusbar.hpp"

void StatusBar::draw()
{
    for (auto &it : _s)
    {
        it.draw_static();
    }
    for (auto &it : _e)
    {
        it->draw_static();
    }
}