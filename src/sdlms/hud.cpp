#include "sdlms/hud.hpp"

Hud::Hud()
{
    _minimap.reset(new MiniMap());
    _statusbar.reset(new StatusBar());
}

void Hud::draw(){
    _minimap->draw();
    _statusbar->draw();
}