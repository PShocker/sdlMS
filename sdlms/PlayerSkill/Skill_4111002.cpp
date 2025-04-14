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

    if (ent != Player::ent)
    {
        return PlayerSkill::SkillResult::None;
    }
    else if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 4111002);
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }

        // 持续时间
        const unsigned int duration = 30000;

        auto buff = World::registry->try_get<Buff>(ent);
        for (auto e : World::registry->view<Summon, Character>())
        {
            auto sum = World::registry->try_get<Summon>(e);
            if (sum->owner == ent && sum->id == 4111002)
            {
                sum->destory = Window::dt_now + duration;
                buff->buffs.at(4111002).destory = sum->destory;
                return;
            }
        }
        // 召唤分身
        auto e = World::registry->create();
        load_character(0, 0, false, u"", e);
        auto sum = &World::registry->emplace<Summon>(e);
        sum->id = 4111002;
        sum->owner = ent;

        Buff::Wrap wrap;
        wrap.duration = duration;
        wrap.destory = Window::dt_now + duration;
        buff->buffs.emplace(4111002, wrap);
    };

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}