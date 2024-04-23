#ifndef HEADER_SDLMS_STATUSBAR
#define HEADER_SDLMS_STATUSBAR

#include <SDL2/SDL.h>
#include <map>
#include <string>
#include <memory>

#include "sdlms/sprite.hpp"
#include "sdlms/button.hpp"
#include "template/currenton.hpp"

class StatusBar : public Currenton<StatusBar>
{
public:
    StatusBar();
    void draw();

public:
    std::vector<Sprite> backgrnd;
    Sprite backgrnd2;
    Sprite chatTarget;
    Sprite quickSlot;

    Button BtShop;
    Button BtMenu;
    Button BtChat;
    Button BtNPT;
};

#endif
