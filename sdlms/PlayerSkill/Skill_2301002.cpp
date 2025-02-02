#include "PlayerSkill.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "Systems/Hit.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 群体治愈
int skill_2301002()
{
    auto ent = Player::ent;
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    AttackWarp atkw;
    atkw.damage = -1000;
    atkw.hit = nullptr;
    atkw.p = &tr->position;
    hit_effect(&atkw, std::nullopt, Player::ent, 3, atkw.damage);

    SkillWarp::cooldowns[u"2301002"] = 1000;
    if (mv->lr)
    {
        return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU;
    }
    else
    {
        return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU | PlayerSkill::SkillResult::ACT;
    }
}