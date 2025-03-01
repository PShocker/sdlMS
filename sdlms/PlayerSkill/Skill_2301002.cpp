#include "PlayerSkill.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "Systems/Hit.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 群体治愈
int skill_2301002(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"2301002");
    auto call_back = [](entt::entity ent)
    {
        auto tr = World::registry->try_get<Transform>(ent);
        AttackWarp atkw;
        atkw.damage = -1000;
        atkw.hit = nullptr;
        atkw.p = tr->position;
        hit_effect(&atkw, std::nullopt, ent, 3, std::nullopt);
    };

    SkillWarp::cooldowns[u"2301002"] = 1500;

    if (state == Character::State::CLIMB)
    {
        call_back(ent);
        return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU;
    }
    else if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }
    ski->call_back = call_back;
    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}