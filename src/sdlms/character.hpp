#ifndef HEADER_SDLMS_CHARACTER
#define HEADER_SDLMS_CHARACTER

#include <cstdint>
#include <vector>
#include <map>

#include <SDL2/SDL.h>
#include "sdlms/dynamicsprite.hpp"
#include "sdlms/input.hpp"

#include "template/currenton.hpp"
#include "template/point.hpp"

class Character : public Currenton<Character>
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
    void event(SDL_Event &event);
    void draw();

public:
    std::map<Status, DynamicSprite> _s;
    Status _status = STAND;
    std::vector<Sprite> _v;
    Point<int> pos={400,80};

private:
    Input *_input;
};

#endif
