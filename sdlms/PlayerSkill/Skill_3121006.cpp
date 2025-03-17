#include "PlayerSkill.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "Entities/Entities.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 火凤凰
int skill_3121006(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    if (mv->foo == nullptr || ent != Player::ent)
    {
        return PlayerSkill::SkillResult::None;
    }
    else
    {
        mv->hspeed = 0;
    }

    // 持续时间
    const unsigned int duration = 30000;

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"3121006");
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }

        auto buff = World::registry->try_get<Buff>(ent);
        auto owner_mv = World::registry->try_get<Move>(ent);
        if (owner_mv->foo)
        {
            for (auto e : World::registry->view<Summon>())
            {
                auto sum = World::registry->try_get<Summon>(e);
                if (sum->state != Summon::State::DIE && sum->owner == ent && sum->id == u"3121006")
                {
                    sum->destory = Window::dt_now + duration;
                    buff->buffs.at(u"3121006").destory = sum->destory;
                    return;
                }
            }
            Skill *ski = World::registry->try_get<Skill>(ent);
            load_summon(ski->skiw->node->get_child(u"summon"), u"3121006", ent);

            Buff::Info info;
            info.duration = duration;
            info.destory = Window::dt_now + duration;
            buff->buffs.emplace(u"3121006", info);
        }
    };
    SkillWarp::cooldowns[u"3121006"] = 1500;
    return PlayerSkill::SkillResult::SOU | PlayerSkill::SkillResult::ACT;
}