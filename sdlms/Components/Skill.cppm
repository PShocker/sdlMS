module;

#include "wz/Property.hpp"

export module components:skill;

import :animatedsprite;

export struct Skill
{
    static inline std::unordered_map<std::u16string, std::vector<AnimatedSprite *>> skill_map;

    std::vector<bool> animated;
    std::u16string id;

    static void init();
    Skill(const std::u16string &id);
    Skill() = default;
};
