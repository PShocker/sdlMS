#ifndef HEADER_SDLMS_CURSOR
#define HEADER_SDLMS_CURSOR

#include <cstdint>
#include <vector>
#include <map>

#include <SDL2/SDL.h>
#include "template/currenton.hpp"
#include "sdlms/dynamicsprite.hpp"
#include "sdlms/input.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/eventsprite.hpp"

class Cursor : public Currenton<Cursor>
{

public:
    enum Event : uint8_t
    {
        NORMAL,
        PRESSED,
        // DISABLED,
        // MOUSEOVER,
        // KEYFOCUSED,
    };

    static inline const std::map<std::u16string, uint8_t> EventMap = {
        {u"0", NORMAL},
        {u"12", PRESSED},
        // {u"disabled", DISABLED},
        // {u"mouseOver", MOUSEOVER},
        // {u"keyfocused", KEYFOCUSED},
    };

public:
    Cursor();
    void load();
    void event(SDL_Event &event);

public:
    EventSprite _s;
    Event _event = NORMAL;

private:
    Camera *_camera;
    Input *_input;

private:
    std::unordered_map<uint8_t, std::vector<SDL_Cursor *>> _c;
};

#endif
