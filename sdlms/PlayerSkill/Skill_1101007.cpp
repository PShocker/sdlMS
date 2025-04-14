#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Commons/Commons.h"
#include "Systems/Attack.h"
#include "Resources/Wz.h"

// 伤害反击
int skill_1101007(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 1101007);
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 1))
        {
            return;
        }
        // 持续时间
        const unsigned int duration = 60000;

        auto buff = World::registry->try_get<Buff>(ent);
        if (buff->buffs.contains(1101007))
        {
            buff->buffs.at(1101007).destory = Window::dt_now + duration;
        }
        else
        {
            Buff::Wrap wrap;
            wrap.after_hit = [](Attack *atk, entt::entity src, entt::entity target, int full_damage)
            {
                // 伤害反馈
                Attack attack;
                attack.damage = full_damage;
                attack.hit = AnimatedSprite::Wrap::load(Wz::Skill->get_root()->find_from_path(u"112.img/skill/1121002/special"));
                attack.src_point = World::registry->try_get<Transform>(target)->position;
                attack.min_damage = 1;
                attack.max_damage = 1;
                attack.mobCount = 0;
                attack_mob(&attack, target, src, std::nullopt);
            };
            wrap.duration = duration;
            wrap.destory = Window::dt_now + wrap.duration;
            buff->buffs.emplace(1101007, wrap);
        }
    };

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}