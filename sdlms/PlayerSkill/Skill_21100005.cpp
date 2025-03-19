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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"21100005");
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }

        // 持续时间
        const unsigned int duration = 60000;

        auto buff = World::registry->try_get<Buff>(ent);
        if (buff->buffs.contains(u"21100005"))
        {
            buff->buffs.at(u"21100005").destory = Window::dt_now + duration;
        }
        else
        {
            Buff::Info info;
            info.after_attack = [](Attack *atk, entt::entity src, entt::entity target)
            {
                Attack attack;
                attack.damage = -attack.damage * 0.1;
                attack_character(&attack, entt::null, src, std::nullopt);
            };
            info.duration = duration;
            info.destory = Window::dt_now + duration;
            buff->buffs.emplace(u"21100005", info);
        }
    };

    if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }
    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}