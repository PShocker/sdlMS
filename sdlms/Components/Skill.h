#pragma once

#include "Sound.h"
#include "Attack.h"
#include "AnimatedSprite.h"
#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include <optional>

struct SkillWarp
{
    struct Info
    {
        int mpCon;
        int damage;
        int mobCount = 1;
        int attackCount = 1;
        std::u16string action;
        SDL_FPoint lt = SDL_FPoint{-110, -32};
        SDL_FPoint rb = SDL_FPoint{-40, -11};

        Info(wz::Node *node);
    };

    std::vector<AnimatedSpriteWarp *> effects;
    std::vector<AnimatedSpriteWarp *> hits;
    std::vector<Info> infos;
    std::u16string id;
    std::optional<std::u16string> action_str = std::nullopt;

    std::unordered_map<std::u16string, SoundWarp *> sounds;
    static inline std::unordered_map<std::u16string, int> cooldowns;

    AnimatedSpriteWarp *ball = nullptr;

    SkillWarp(const std::u16string &id);
    SkillWarp() = default;

    static SkillWarp *load(const std::u16string &id);
    static void clean_up();
};

struct Skill
{
    SkillWarp *ski = nullptr;
    std::optional<AttackWarp> atkw = std::nullopt;

    Skill(const std::u16string &id);
    Skill() = default;

    bool hit = false;
    uint8_t level = 19;
    bool attack = false;
    unsigned char ball = 0;
};
