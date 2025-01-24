#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 金钱炸弹
int skill_4211006()
{
    Skill ski(u"4211006");

    auto ent = Player::ent;

    PlayerSkill::skill_attack(&ski);

    SkillWarp::cooldowns[u"4211006"] = 500;

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU | PlayerSkill::SkillResult::ACT;
}