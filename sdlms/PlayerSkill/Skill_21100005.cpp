#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <numbers>
#include "Commons/Commons.h"
#include "Systems/Attack.h"

// 连环吸血
int skill_21100005(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 21100005);
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }

        // 持续时间
        const unsigned int duration = 60000;

        auto buff = World::registry->try_get<Buff>(ent);
        if (buff->buffs.contains(21100005))
        {
            buff->buffs.at(21100005).destory = Window::dt_now + duration;
        }
        else
        {
            Buff::Wrap wrap;
            wrap.start = [](entt::entity src)
            {
                auto buff = World::registry->try_get<Buff>(src);
                auto &wrap = buff->buffs[21100005];
                wrap.data = 0;
            };
            wrap.after_attack = [](Attack *atk, entt::entity src, entt::entity target, int full_damage)
            {
                auto buff = World::registry->try_get<Buff>(src);
                auto &wrap = buff->buffs[21100005];
                wrap.data = std::any_cast<int>(wrap.data) + full_damage;
                if (atk->mobCount == 0)
                {
                    Attack attack;
                    attack.damage = -std::any_cast<int>(wrap.data);
                    attack_player(&attack, entt::null, std::nullopt);
                    wrap.data = 0;
                }
            };
            wrap.duration = duration;
            wrap.destory = Window::dt_now + duration;
            buff->buffs.emplace(21100005, wrap);
            wrap.start.value()(ent);
        }
    };

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}