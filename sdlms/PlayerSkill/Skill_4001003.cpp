#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Entities/Entities.h"
#include "Components/Components.h"

// 隐身术
int skill_4001003(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 4001003);
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }
        // 持续时间
        const unsigned int duration = 25000;

        auto buff = World::registry->try_get<Buff>(ent);
        if (buff->buffs.contains(4001003))
        {
            buff->buffs.at(4001003).destory = Window::dt_now + duration;
        }
        Buff::Wrap wrap;
        wrap.start = [](entt::entity src)
        {
            auto cha = World::registry->try_get<Character>(src);
            cha->alpha = 128;
            cha->invincible_cooldown = Window::dt_now + 2000;
        };
        wrap.frame = wrap.start;
        wrap.finish = [](entt::entity src)
        {
            auto cha = World::registry->try_get<Character>(src);
            cha->alpha = 255;
            cha->invincible_cooldown = Window::dt_now;
        };
        wrap.duration = duration;
        wrap.destory = Window::dt_now + duration;
        buff->buffs.emplace(4001003, wrap);
        wrap.start.value()(ent);
    };

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}