#ifndef HEADER_SDLMS_EVENTSPRITE
#define HEADER_SDLMS_EVENTSPRITE

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <variant>

#include "sdlms/animatedsprite.hpp"
#include "sdlms/sprite.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/dynamicsprite.hpp"
#include "sdlms/input.hpp"

class EventSprite
{
public:
    enum Event
    {
        NORMAL,
        PRESSED,
        DISABLED,
        MOUSEOVER,
        KEYFOCUSED,
    };

    static inline const std::map<std::u16string, Event> EventMap = {
        {u"normal", NORMAL},
        {u"pressed", PRESSED},
        {u"disabled", DISABLED},
        {u"mouseOver", MOUSEOVER},
        {u"keyfocused", KEYFOCUSED},
    };

public:
    EventSprite(std::map<Event, DynamicSprite> eventsprite);
    void event(SDL_Event &event);

    void draw();
    void _draw();
    void update(int elapsedTime);
    SDL_FRect rect();

public:
    std::map<Event, DynamicSprite> _eventsprite;
    Event _event = NORMAL;

private:
    Camera *_camera;
    Input *_input;
};
#endif