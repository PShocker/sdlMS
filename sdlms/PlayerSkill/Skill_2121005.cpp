#include "PlayerSkill.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "Entities/Entities.h"
#include "Systems/Hit.h"
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
    auto call_back = [](entt::entity ent)
    {
        auto o_mv = World::registry->try_get<Move>(ent);
        if (o_mv->foo)
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
            auto e = World::registry->create();
            auto tr = World::registry->try_get<Transform>(ent);
            Skill *ski = World::registry->try_get<Skill>(ent);
            load_summon(ski->skiw->node->get_child(u"summon"), e, tr->position.x, tr->position.y, tr->z - 2, u"2121005", ent);
            World::registry->emplace<Effect>(e);
            auto s_mv = &World::registry->emplace<Move>(e);
            s_mv->foo = o_mv->foo;
        }
    };
    ski->call_back = call_back;
    SkillWarp::cooldowns[u"2121005"] = 1500;
    return PlayerSkill::SkillResult::SOU | PlayerSkill::SkillResult::ACT;
}