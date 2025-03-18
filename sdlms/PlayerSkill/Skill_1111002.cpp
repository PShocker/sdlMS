#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 斗气
int skill_1111002(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"1111002");
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }

        // 持续时间
        const unsigned int duration = -1;

        auto buff = World::registry->try_get<Buff>(ent);
        if (buff->buffs.contains(u"1111002"))
        {
            buff->buffs.at(u"1111002").destory = Window::dt_now;
        }
        else
        {
            Buff::Info info;
            info.after_attack = [](entt::entity src, entt::entity target)
            {
                auto buff = World::registry->try_get<Buff>(src);
                auto &info = buff->buffs[u"1111002"];
                // auto eff = World::registry->try_get<Effect>(ent);

                // eff->effects.push_back({std::nullopt, AnimatedSprite(AnimatedSpriteWarp::load(ski->skiw->node->find_from_path(u"CharLevel/25/effect")))});

                if (!info.data.has_value())
                {
                    info.data = std::map<int, SDL_FPoint>();
                    auto val = std::any_cast<std::map<int, SDL_FPoint>>(info.data);
                    val.emplace(1, SDL_FPoint{-10, -10});
                }
                else
                {
                    // int val = std::any_cast<int>(info.data);
                    // info.data = val + 1;
                }
            };
            info.frame = [](entt::entity src)
            {
                auto buff = World::registry->try_get<Buff>(src);
                auto &info = buff->buffs[u"1111002"];
                if (!info.data.has_value())
                {
                    info.data = 1;
                }
                else
                {
                    int val = std::any_cast<int>(info.data);
                    info.data = val + 1;
                }
            };
            info.duration = duration;
            info.destory = 0;
            buff->buffs.emplace(u"1111002", info);
        }
    };

    if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}