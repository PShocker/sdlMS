#pragma once

#include "Sound.h"
#include "Attack.h"
#include "AnimatedSprite.h"
#include "wz/Property.hpp"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>
#include <functional>
#include <any>

struct SkillWarp
{
    int id;
    std::vector<AnimatedSpriteWarp *> effects;
    std::vector<AnimatedSpriteWarp *> hits;
    std::vector<wz::Node *> level;
    std::optional<std::u16string> action_str = std::nullopt;

    std::unordered_map<std::u16string, SoundWarp *> sounds;
    static inline std::unordered_map<int, int> cooldowns;

    wz::Node *node = nullptr;

    SkillWarp(int id);
    SkillWarp() = default;

    static SkillWarp *load(int id);
};

struct Skill
{
    SkillWarp *skiw = nullptr;
    std::optional<Attack> atk = std::nullopt;

    Skill(int id);
    Skill() = default;

    bool hit = false;
    uint8_t level = 19;
    bool attack = false;

    // 数据
    std::any data;

    std::unordered_set<entt::entity> hit_targets;
    std::optional<std::function<void(entt::entity, int, int)>> call_back = std::nullopt;
};
