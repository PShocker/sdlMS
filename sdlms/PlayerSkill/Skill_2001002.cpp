#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Commons/Commons.h"

// 魔法盾
int skill_2001002(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    if (ent != Player::ent)
    {
        return PlayerSkill::SkillResult::None;
    }
    else if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 2001002);
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }

        // 持续时间
        const unsigned int duration = 0;

        auto buff = World::registry->try_get<Buff>(ent);
        if (buff->buffs.contains(2001002))
        {
            buff->buffs.at(2001002).destory = Window::dt_now;
        }
        else
        {
            Buff::Wrap wrap;
            wrap.before_attack = [](Attack *atk, entt::entity src)
            {
                atk->min_damage = 0.3;
                atk->max_damage = 0.3;
            };
            wrap.duration = duration;
            wrap.destory = 0;
            buff->buffs.emplace(2001002, wrap);
        }
    };

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}