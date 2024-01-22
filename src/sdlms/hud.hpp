#ifndef HEADER_SDLMS_HUD
#define HEADER_SDLMS_HUD

#include <SDL2/SDL.h>
#include "util/currenton.hpp"
#include "sdlms/graphics.hpp"
#include "sdlms/minimap.hpp"

class Hud : public Currenton<Hud>
{
public:
    Hud();

public:
    std::unique_ptr<MiniMap> _minimap;
};

#endif