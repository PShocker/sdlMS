#ifndef HEADER_SDLMS_CHARACTER
#define HEADER_SDLMS_CHARACTER

#include <cstdint>
#include <vector>
#include <map>

#include <SDL2/SDL.h>
#include "sdlms/dynamicsprite.hpp"
#include "sdlms/input.hpp"
#include "sdlms/graphics.hpp"

#include "template/currenton.hpp"
#include "template/point.hpp"

class Character : public Currenton<Character>
{

public:
    enum Type : uint8_t
    {
        ALERT,
        DEAD,
        FLY,
        HEAL,
        JUMP,
        LADDER,
        PRONE,
        PRONESTAB,
        ROPE,
        SHOT,
        SHOOT1,
        SHOOT2,
        SHOOTF,
        SIT,
        STABO1,
        STABO2,
        STABOF,
        STABT1,
        STABT2,
        STABTF,
        STAND1,
        STAND2,
        SWINGO1,
        SWINGO2,
        SWINGO3,
        SWINGOF,
        SWINGP1,
        SWINGP2,
        SWINGPF,
        SWINGT1,
        SWINGT2,
        SWINGT3,
        SWINGTF,
        WALK1,
        WALK2,
        LENGTH,
    };

public:
    Character();
    void event(SDL_Event &event);
    void draw();
    void update(int elapsedTime);

public:
    std::unordered_map<uint8_t, std::vector<std::tuple<std::vector<Sprite>, int>>> _s;
    Type _status = WALK1;
    std::vector<Sprite> _v;
    Point<float> _pos = {0, 0};
    int _frameIndex = 0;
    int _frameTime = 0;
    bool _direct=true;

private:
    Input *_input;
    SDL_Renderer *_renderer;
};

#endif
