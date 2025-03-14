#include "PlayerSkill.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "Entities/Entities.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 冰魔兽
int skill_2121005(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    if (mv->foo == nullptr)
    {
        return PlayerSkill::SkillResult::None;
    }
    else
    {
        mv->hspeed = 0;
    }

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"2121005");
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }
        auto owner_mv = World::registry->try_get<Move>(ent);
        if (owner_mv->foo)
        {
            for (auto e : World::registry->view<Summon>())
            {
                auto sum = World::registry->try_get<Summon>(e);
                if (sum->owner == ent && sum->id == u"2121005")
                {
                    sum->destory = Window::dt_now + 30000;
                    return;
                }
            }
            Skill *ski = World::registry->try_get<Skill>(ent);
            auto e = load_summon(ski->skiw->node->get_child(u"summon"), u"2121005", ent);
            auto summon_mv = World::registry->try_get<Move>(e);
            summon_mv->foo = owner_mv->foo;
        }
    };
    SkillWarp::cooldowns[u"2121005"] = 1500;
    return PlayerSkill::SkillResult::SOU | PlayerSkill::SkillResult::ACT;
}