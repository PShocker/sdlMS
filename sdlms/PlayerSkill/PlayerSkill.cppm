module;

#include <string>
#include <functional>

export module playerskill;

import components;

export struct PlayerSkill
{
    static void init();

    static void skill_sound(SkillWarp *souw, int delay = 0);

    static void skill_effect(Skill *ski);

    static void skill_action(Skill *ski);

    static void skill_attack(Skill *ski, int damage = 50);

    static inline std::unordered_map<std::u16string, std::function<int()>> Skills;

    enum SkillResult
    {
        None = -1,
        EFF = 1,
        SOU = 2,
        ATK = 4,
        ACT = 8
    };
};

int skill_2201002();
int skill_4211006();