#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Entities/Entities.h"
#include "Components/Components.h"

// 分身术
int skill_4111002(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"4111002");
    auto call_back = [](entt::entity ent)
    {
        for (auto e : World::registry->view<Summon, Character>())
        {
            auto sum = World::registry->try_get<Summon>(e);
            if (sum->owner == ent)
            {
                sum->destory = Window::dt_now + 30000;
                return;
            }
        }
        // 召唤分身
        auto e = World::registry->create();
        load_character(0, 0, true, e);
        auto sum = &World::registry->emplace<Summon>(e);
        sum->id = u"4111002";
        sum->owner = ent;
    };

    SkillWarp::cooldowns[u"4111002"] = 1000;

    if (state == Character::State::CLIMB)
    {
        call_back(ent);
        return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU;
    }
    else if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }
    ski->call_back = call_back;
    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}