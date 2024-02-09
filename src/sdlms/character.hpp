#ifndef HEADER_SDLMS_PLAYER
#define HEADER_SDLMS_PLAYER

#include <cstdint>
#include <vector>
#include <map>

#include <SDL2/SDL.h>
#include "sdlms/dynamicsprite.hpp"
#include "sdlms/input.hpp"

class Character
{

public:
    enum Status
    {
        STAND,
        WALK,
        // DISABLED,
        // MOUSEOVER,
        // KEYFOCUSED,
    };

public:
    Character();
    void load();
    void event(SDL_Event &event);

public:
    std::map<Status, DynamicSprite> _s;
    Status _status = STAND;

private:
    Camera *_camera;
    Input *_input;
};

#endif
