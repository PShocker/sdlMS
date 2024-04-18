#ifndef HEADER_SDLMS_STATUSBAR
#define HEADER_SDLMS_STATUSBAR

#include <SDL2/SDL.h>
#include <map>
#include <string>
#include <memory>

#include "sdlms/sprite.hpp"
#include "sdlms/uisprite.hpp"
#include "template/currenton.hpp"

class StatusBar : public Currenton<StatusBar>
{
public:
    void draw();

public:
    std::vector<Sprite> _s;
    std::vector<UISprite *> _e;
};

#endif
