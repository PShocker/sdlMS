#ifndef HEADER_SDLMS_EVENTSPRITE
#define HEADER_SDLMS_EVENTSPRITE

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <variant>

#include "sdlms/animatedsprite.hpp"
#include "sdlms/sprite.hpp"
#include "sdlms/dynamicsprite.hpp"
#include "sdlms/input.hpp"

class EventSprite
{
public:
    EventSprite(){};
    EventSprite(std::unordered_map<uint8_t, DynamicSprite> eventsprite);
    virtual ~EventSprite();

    void draw(uint8_t i);
    void draw_static(uint8_t i);
    bool update(uint8_t i, int elapsedTime);
    SDL_FRect rect(uint8_t i);

public:
    static EventSprite load_event_sprite(std::map<std::u16string, uint8_t> event_map, wz::Node *node, int x = 0, int y = 0);

public:
    std::unordered_map<uint8_t, DynamicSprite> _eventsprite;
};
#endif