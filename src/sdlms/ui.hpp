#ifndef HEADER_SDLMS_UI
#define HEADER_SDLMS_UI

#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/eventsprite.hpp"

class UI : public EventSprite
{
public:
    enum Event : uint8_t
    {
        NORMAL,
        PRESSED,
        DISABLED,
        MOUSEOVER,
        KEYFOCUSED,
    };

    static inline const std::map<std::u16string, uint8_t> EventMap = {
        {u"normal", NORMAL},
        {u"pressed", PRESSED},
        {u"disabled", DISABLED},
        {u"mouseOver", MOUSEOVER},
        {u"keyfocused", KEYFOCUSED},
    };

    UI(EventSprite eventsprite);
    void draw();
    void _draw();
    SDL_FRect rect();
    void event(SDL_Event &event);

public:
    int _event = NORMAL;

private:
    Input *_input;
};
#endif