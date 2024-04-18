#ifndef HEADER_SDLMS_UISPRITE
#define HEADER_SDLMS_UISPRITE

#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/eventsprite.hpp"

class UISprite : public EventSprite
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

    UISprite(EventSprite eventsprite);
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