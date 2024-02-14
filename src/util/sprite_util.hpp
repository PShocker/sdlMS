#ifndef HEADER_UTIL_SPRITE
#define HEADER_UTIL_SPRITE

#include <vector>
#include <ranges>
#include "wz/Property.hpp"

#include "template/currenton.hpp"

#include "sdlms/sprite.hpp"
#include "sdlms/animatedsprite.hpp"
#include "sdlms/eventsprite.hpp"
//
namespace util
{
    class SpriteUtil final : public Currenton<SpriteUtil>
    {
    public:
        Sprite load_sprite(wz::Node *node, int x = 0, int y = 0, int flip = SDL_FLIP_NONE);
        AnimatedSprite load_animated_sprite(wz::Node *node, int x = 0, int y = 0, int flip = SDL_FLIP_NONE);
        EventSprite *load_event_sprite(wz::Node *node, int x = 0, int y = 0);
    };
}

#endif