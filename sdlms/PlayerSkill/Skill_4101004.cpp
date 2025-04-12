#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 轻功
int skill_4101004(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 4101004);
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }

        // 持续时间
        const unsigned int duration = 60000;

        auto buff = World::registry->try_get<Buff>(ent);
        if (buff->buffs.contains(4101004))
        {
            buff->buffs.at(4101004).destory = Window::dt_now + duration;
        }
        else
        {
            Buff::Wrap wrap;
            wrap.duration = duration;
            wrap.start = [](entt::entity ent)
            {
                auto mv = World::registry->try_get<Move>(ent);
                mv->hspeed_max.value() += 80;
                mv->hspeed_min.value() -= 80;
                mv->vspeed_min.value() -= 888;
            };
            wrap.finish = [](entt::entity ent)
            {
                auto mv = World::registry->try_get<Move>(ent);
                mv->hspeed_max.value() -= 80;
                mv->hspeed_min.value() += 80;
                mv->vspeed_min.value() += 888;
            };
            wrap.destory = Window::dt_now + duration;
            buff->buffs.emplace(4101004, wrap);
            wrap.start.value()(ent);
        }
    };

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}