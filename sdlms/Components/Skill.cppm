module;

#include "wz/Property.hpp"
#include <SDL3/SDL.h>

export module components:skill;

import :animatedsprite;

export struct Skill
{
    struct Info
    {
        int mpCon;
        int damage;
        int mobCount;
        int attackCount;
        std::u16string action;
        SDL_FPoint lt;
        SDL_FPoint rb;

        Info(wz::Node *node);
    };

    std::vector<AnimatedSpriteWarp *> effects;
    std::vector<AnimatedSpriteWarp *> hits;
    std::vector<Info *> infos;
    std::u16string id;

    Skill(const std::u16string &id);
    Skill() = default;

    static Skill *load(const std::u16string &id);
};

export Skill *load_skill(const std::u16string &id);
