#include "PlayerSkill.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Systems/Attack.h"

// 群体治愈
int skill_2301002(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"2301002");
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }
        auto tr = World::registry->try_get<Transform>(ent);
        Attack atk;
        atk.damage = -1000;
        atk.hit = nullptr;
        atk.src_point = tr->position;
        attack_player(&atk, entt::null, ent, std::nullopt);
    };

    SkillWarp::cooldowns[u"2301002"] = 1500;

    if (state == Character::State::CLIMB)
    {
        ski->call_back.value()(ent, 0, 0);
        return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU;
    }
    else if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }
    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}