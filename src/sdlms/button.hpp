#ifndef HEADER_SDLMS_UI
#define HEADER_SDLMS_UI

#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/eventsprite.hpp"

class Button
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

    Button(){};
    Button(EventSprite eventsprite, std::u16string event_key);
    Button& operator=(Button&& other) noexcept;
    ~Button();
    void draw();
    void draw_static();
    SDL_FRect rect();
    void event(SDL_Event &event);

public:
    EventSprite _es;
    int _event = NORMAL;
    std::u16string _event_key;

private:
    Input *_input;
};
#endif