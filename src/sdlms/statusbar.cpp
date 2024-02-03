#include "sdlms/statusbar.hpp"


void StatusBar::draw()
{
    for (auto &it : _s)
    {
        it._draw();
    }
    for (auto &it : _e)
    {
        it->draw();
    }
}