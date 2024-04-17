#ifndef HEADER_SDLMS_NPC
#define HEADER_SDLMS_NPC

#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/sprite.hpp"

class Npc
{
public:
    enum Event
    {
        STAND,
        SHY,
        // DISABLED,
        // MOUSEOVER,
        // KEYFOCUSED,
    };

    static inline const std::map<std::u16string, Event> EventMap = {
        {u"stand", STAND},
        {u"shy", SHY},
        // {u"disabled", DISABLED},
        // {u"mouseOver", MOUSEOVER},
        // {u"keyfocused", KEYFOCUSED},
    };

public:
    Npc(std::unordered_map<Event, DynamicSprite> eventsprite, int fh);

    static void draws(std::vector<Npc> &npcs);

public:
    std::unordered_map<Event, DynamicSprite> _eventsprite;
    Event _event = STAND;
    int _fh;
};
#endif