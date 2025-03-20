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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"1101004");
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }

        // 持续时间
        const unsigned int duration = 60000;

        auto buff = World::registry->try_get<Buff>(ent);
        if (buff->buffs.contains(u"1101004"))
        {
            buff->buffs.at(u"1101004").destory = Window::dt_now + duration;
        }
        else
        {
            Buff::Info info;
            info.duration = duration;
            info.start = [](entt::entity ent)
            {
                // 遍历 map，将每个 value 改为原来的 0.8 倍
                for (int i = 0; i < Character::stance_delays->size(); i++)
                {
                    if (i == Character::ACTION::ALERT ||
                        Character::ACTION::SIT ||
                        Character::ACTION::STAND1 ||
                        Character::ACTION::STAND2 ||
                        Character::ACTION::LADDER ||
                        Character::ACTION::ROPE ||
                        Character::ACTION::DEAD ||
                        Character::ACTION::WALK1 ||
                        Character::ACTION::WALK2)
                    {
                        continue;
                    }
                    for (auto &pair : Character::stance_delays[i])
                    {
                        pair.second = pair.second * 0.8; // 转换回 int32_t
                    }
                }
            };
            info.finish = [](entt::entity ent)
            {
                // 还原
                // 遍历 map，将每个 value 改为原来的 0.8 倍
                for (int i = 0; i < Character::stance_delays->size(); i++)
                {
                    if (i == Character::ACTION::ALERT ||
                        Character::ACTION::SIT ||
                        Character::ACTION::STAND1 ||
                        Character::ACTION::STAND2 ||
                        Character::ACTION::LADDER ||
                        Character::ACTION::ROPE ||
                        Character::ACTION::DEAD ||
                        Character::ACTION::WALK1 ||
                        Character::ACTION::WALK2)
                    {
                        continue;
                    }
                    for (auto &pair : Character::stance_delays[i])
                    {
                        pair.second = pair.second * 1.25; // 转换回 int32_t
                    }
                }
            };
            info.destory = Window::dt_now + duration;
            buff->buffs.emplace(u"1101004", info);
            info.start.value()(ent);
        }
    };

    if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}