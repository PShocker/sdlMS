module;

#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include <optional>

export module components:skill;

import :animatedsprite;

export struct SkillWarp
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
    std::optional<std::u16string> action_str=std::nullopt;

    SkillWarp(const std::u16string &id);
    SkillWarp() = default;

    static SkillWarp *load(const std::u16string &id);
};

export struct Skill
{
    SkillWarp *ski = nullptr;
    Skill(const std::u16string &id);
    Skill() = default;
};
