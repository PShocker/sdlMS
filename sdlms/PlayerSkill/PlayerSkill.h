#pragma once

#include <string>
#include <any>
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
int skill_4211002(entt::entity ent);
int skill_4201004(entt::entity ent);
int skill_3101005(entt::entity ent);
int skill_1121008(entt::entity ent);
int skill_4201005(entt::entity ent);
int skill_4121007(entt::entity ent);
int skill_1121006(entt::entity ent);
int skill_2101005(entt::entity ent);
int skill_2201004(entt::entity ent);
int skill_2211002(entt::entity ent);
int skill_4101004(entt::entity ent);
int skill_1111002(entt::entity ent);
int skill_2001002(entt::entity ent);
int skill_21100005(entt::entity ent);
int skill_14101006(entt::entity ent);
int skill_1101004(entt::entity ent);
int skill_1111008(entt::entity ent);
int skill_4001003(entt::entity ent);
int skill_1101007(entt::entity ent);
int skill_3111006(entt::entity ent);
int skill_3111003(entt::entity ent);
int skill_2101004(entt::entity ent);
int skill_3221005(entt::entity ent);
int skill_3001005(entt::entity ent);

struct PlayerSkill
{
    static void skill_sound(Skill *ski, int delay = 0);

    static void skill_effect(Skill *ski, entt::entity ent);

    static void skill_action(Skill *ski, entt::entity ent);

    static void skill_attack(Skill *ski);

    static const inline std::unordered_map<int, std::function<int(entt::entity)>> Skills = {
        {1001004, skill_1001004},
        {1001005, skill_1001005},
        {2201002, skill_2201002},
        {2121005, skill_2121005},
        {2221005, skill_2221005},
        {2221006, skill_2221006},
        {2301002, skill_2301002},
        {3121006, skill_3121006},
        {4211006, skill_4211006},
        {1311006, skill_1311006},
        {4001344, skill_4001344},
        {4111002, skill_4111002},
        {4211002, skill_4211002},
        {4201004, skill_4201004},
        {3101005, skill_3101005},
        {1121008, skill_1121008},
        {4201005, skill_4201005},
        {4121007, skill_4121007},
        {1121006, skill_1121006},
        {2101005, skill_2101005},
        {2201004, skill_2201004},
        {2211002, skill_2211002},
        {4101004, skill_4101004},
        {1111002, skill_1111002},
        {2001002, skill_2001002},
        {21100005, skill_21100005},
        {14101006, skill_14101006},
        {1101004, skill_1101004},
        {1111008, skill_1111008},
        {4001003, skill_4001003},
        {1101007, skill_1101007},
        {3111006, skill_3111006},
        {3111003, skill_3111003},
        {2101004, skill_2101004},
        {3221005, skill_3221005},
        {3001005, skill_3001005},
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
