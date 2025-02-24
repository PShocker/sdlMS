#include "PlayerSkill.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "Entities/Entities.h"
#include "Systems/Hit.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 火魔兽
int skill_2221005(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"2221005");
    auto call_back = [](entt::entity ent)
    {
        for (auto e : World::registry->view<Summon>())
        {
            auto sum = World::registry->try_get<Summon>(e);
            if (sum->owner == ent && sum->id == u"2221005")
            {
                sum->destory = Window::dt_now + 30000;
                return;
            }
        }
        auto e = World::registry->create();
        auto tr = World::registry->try_get<Transform>(ent);
        Skill *ski = World::registry->try_get<Skill>(ent);
        load_summon(ski->skiw->summon, e, tr->position.x, tr->position.y, tr->z - 2);
        auto sum = World::registry->try_get<Summon>(e);
        sum->id = u"2221005";
        sum->owner = ent;
    };

    SkillWarp::cooldowns[u"2221005"] = 1500;

    if (state == Character::State::CLIMB)
    {
        call_back(ent);
        return PlayerSkill::SkillResult::SOU;
    }
    else if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }
    ski->call_back = call_back;
    return PlayerSkill::SkillResult::SOU | PlayerSkill::SkillResult::ACT;
}