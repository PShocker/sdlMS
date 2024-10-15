module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

module playerskill;

import components;
import core;
import commons;

// 金钱炸弹
int skill_4211006()
{
    Skill ski(u"4211006");

    auto ent = Player::ent;

    PlayerSkill::skill_attack(&ski, 1);

    SkillWarp::cooldowns[u"4211006"] = 500;

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU | PlayerSkill::SkillResult::ACT;
}