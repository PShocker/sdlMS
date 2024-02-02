#ifndef HEADER_UTIL_SPRITE
#define HEADER_UTIL_SPRITE

#include <vector>
#include <ranges>
#include "wz/Property.hpp"

#include "util/currenton.hpp"

#include "sdlms/sprite.hpp"
#include "sdlms/animatedsprite.hpp"

//
namespace util
{
    class SpriteUtil final : public Currenton<SpriteUtil>
    {
    public:
        Sprite load_sprite(wz::Node *node, int x = 0, int y = 0);
    };
}

#endif