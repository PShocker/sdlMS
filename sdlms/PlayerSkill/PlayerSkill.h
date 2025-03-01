#pragma once

#include <string>
#include <functional>
#include "entt/entt.hpp"
#include "Components/Components.h"

int skill_1001004(entt::entity ent);
int skill_1001005(entt::entity ent);
int skill_2201002(entt::entity ent);
int skill_2121005(entt::entity ent);
int skill_2221005(entt::entity ent);
int skill_2221006(entt::entity ent);
int skill_2301002(entt::entity ent);
int skill_3121006(entt::entity ent);
int skill_4211006(entt::entity ent);
int skill_1311006(entt::entity ent);
int skill_4001344(entt::entity ent);
int skill_4111002(entt::entity ent);

struct PlayerSkill
{
    static void skill_sound(SkillWarp *souw, int delay = 0);

    static void skill_effect(Skill *ski, entt::entity ent);

    static void skill_action(Skill *ski, entt::entity ent);

    static void skill_attack(Skill *ski);

    static const inline std::unordered_map<std::u16string, std::function<int(entt::entity)>> Skills = {
        {u"1001004", skill_1001004},
        {u"1001005", skill_1001005},
        {u"2201002", skill_2201002},
        {u"2121005", skill_2121005},
        {u"2221005", skill_2221005},
        {u"2221006", skill_2221006},
        {u"2301002", skill_2301002},
        {u"3121006", skill_3121006},
        {u"4211006", skill_4211006},
        {u"1311006", skill_1311006},
        {u"4001344", skill_4001344},
        {u"4111002", skill_4111002},
    };

    enum SkillResult
    {
        None = -1,
        EFF = 1,
        SOU = 2,
        ATK = 4,
        ACT = 8,
        ALERT = 16
    };
};
