#ifndef HEADER_UTIL_CURSOR
#define HEADER_UTIL_CURSOR

#include <vector>
#include <ranges>
#include <map>

#include "wz/Property.hpp"

#include "util/currenton.hpp"
#include "util/sprite_util.hpp"

#include "sdlms/character.hpp"


class Character;

namespace util
{
    class CharacterUtil final : public Currenton<CharacterUtil>
    {
    public:
        CharacterUtil();
        std::map<Character::Status, DynamicSprite> load();

    private:
        SpriteUtil *_sprite_util;

    private:
        wz::Node *_character_node;
    
    private:
        void load_body(wz::Node *node);
    };
}

#endif