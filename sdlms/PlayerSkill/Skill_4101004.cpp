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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"4101004");
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }

        // 持续时间
        const unsigned int duration = 60000;

        auto buff = World::registry->try_get<Buff>(ent);
        if (buff->buffs.contains(u"4101004"))
        {
            buff->buffs.at(u"4101004").destory = Window::dt_now + duration;
        }
        else
        {
            Buff::Info info;
            info.duration = duration;
            info.start = [](entt::entity ent)
            {
                auto mv = World::registry->try_get<Move>(ent);
                mv->hspeed_max.value() += 80;
                mv->hspeed_min.value() -= 80;
                mv->vspeed_min.value() -= 888;
            };
            info.finish = [](entt::entity ent)
            {
                auto mv = World::registry->try_get<Move>(ent);
                mv->hspeed_max.value() -= 80;
                mv->hspeed_min.value() += 80;
                mv->vspeed_min.value() += 888;
            };
            info.start.value()(ent);
            info.destory = Window::dt_now + duration;
            buff->buffs.emplace(u"4101004", info);
        }
    };

    if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}