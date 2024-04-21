#ifndef HEADER_SDLMS_NPC
#define HEADER_SDLMS_NPC

#include <SDL2/SDL.h>
#include <random>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/sprite.hpp"
#include "sdlms/eventsprite.hpp"

class Npc : public EventSprite
{
public:
    enum Event : uint8_t
    {
        STAND,
        SHY,
        HAND,
        WINK,
        EYE,
        SAY,
    };

    static inline const std::map<std::u16string, uint8_t> EventMap = {
        {u"stand", STAND},
        {u"shy", SHY},
        {u"hand", HAND},
        {u"wink", WINK},
        {u"eye", EYE},
        {u"say", SAY},
    };

public:
    Npc(EventSprite eventsprite, std::u16string id, int fh);
    void draw();
    void update(int elapsedTime);
    static void draws(std::vector<Npc> &npcs);
    static void updates(std::vector<Npc> &npcs, int elapsedTime);

public:
    std::u16string _id;
    uint8_t _event = STAND;
    int _fh;

    Sprite _name;
    Sprite _func;

private:
    std::default_random_engine _gen;
    std::uniform_int_distribution<uint8_t> _dist;
    std::vector<uint8_t> _v_event;
};
#endif