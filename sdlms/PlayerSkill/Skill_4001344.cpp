#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 双飞镖
int skill_4001344()
{
    auto ent = Player::ent;
    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"4001344");
    auto mv = World::registry->try_get<Move>(ent);

    ski->ball = 2;

    SkillWarp::cooldowns[u"4001344"] = 500;
    if (mv->lr)
    {
        return PlayerSkill::SkillResult::None;
    }
    else
    {
        return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
               PlayerSkill::SkillResult::ACT | PlayerSkill::SkillResult::ALERT;
    }
}