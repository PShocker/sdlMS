#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 攻速提升(快速剑，快速斧，快速矛，快速枪...)
int skill_1101004(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 1101004);
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }

        // 持续时间
        const unsigned int duration = 60000;

        auto buff = World::registry->try_get<Buff>(ent);
        if (buff->buffs.contains(1101004))
        {
            buff->buffs.at(1101004).destory = Window::dt_now + duration;
        }
        else
        {
            Buff::Wrap wrap;
            wrap.duration = duration;
            wrap.start = [](entt::entity ent)
            {
                for (int i = 0; i < Character::Action::LENGTH; ++i)
                { // 遍历数组中的每个 unordered_map
                    if (i == Character::Action::ALERT ||
                        i == Character::Action::SIT ||
                        i == Character::Action::STAND1 ||
                        i == Character::Action::STAND2 ||
                        i == Character::Action::LADDER ||
                        i == Character::Action::ROPE ||
                        i == Character::Action::DEAD ||
                        i == Character::Action::WALK1 ||
                        i == Character::Action::WALK2)
                    { // 跳过
                        continue;
                    }
                    for (auto &pair : Character::stance_delays[i])
                    {                       // 遍历 unordered_map 中的每个键值对
                        pair.second *= 0.8; // 将 value 设为原来的 0.5 倍
                    }
                }
                for (auto &outer_pair : Character::body_actions)
                {
                    for (auto &inner_pair : outer_pair.second)
                    {
                        inner_pair.second.delay *= 0.8;
                    }
                }
            };
            wrap.finish = [](entt::entity ent)
            {
                // 还原
                for (int i = 0; i < Character::Action::LENGTH; ++i)
                { // 遍历数组中的每个 unordered_map
                    if (i == Character::Action::ALERT ||
                        i == Character::Action::SIT ||
                        i == Character::Action::STAND1 ||
                        i == Character::Action::STAND2 ||
                        i == Character::Action::LADDER ||
                        i == Character::Action::ROPE ||
                        i == Character::Action::DEAD ||
                        i == Character::Action::WALK1 ||
                        i == Character::Action::WALK2)
                    { // 跳过
                        continue;
                    }
                    for (auto &pair : Character::stance_delays[i])
                    {                        // 遍历 unordered_map 中的每个键值对
                        pair.second *= 1.25; // 将 value 设为原来的 0.5 倍
                    }
                }
                for (auto &outer_pair : Character::body_actions)
                {
                    for (auto &inner_pair : outer_pair.second)
                    {
                        inner_pair.second.delay *= 1.25;
                    }
                }
            };
            wrap.destory = Window::dt_now + duration;
            buff->buffs.emplace(1101004, wrap);
            wrap.start.value()(ent);
        }
    };

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}