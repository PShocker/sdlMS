#include "sdlms/hud.hpp"

Hud::Hud()
{
    _minimap.reset(new MiniMap());
}

void Hud::draw(){
    _minimap->draw();
}