#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Systems/MobStateMachine.h"
#include "Commons/Commons.h"

// 冰冻术
int skill_2201004(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    // 通用攻击技能
    if (state == Character::State::CLIMB)
    {
        return PlayerSkill::SkillResult::None;
    }
    else if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"2201004");

    auto node = ski->skiw->level[ski->level];
    auto lt = SDL_FPoint{-180, -32};
    auto rb = SDL_FPoint{-40, -11};
    auto attackCount = 1;
    auto mobCount = 1;
    auto hit = ski->skiw->hits[0];
    SoundWarp *souw = ski->skiw->sounds[u"Hit"];

    ski->atk = Attack(lt, rb, nullptr, mobCount, attackCount, souw, 30);
    ski->atk.value().call_back = [hit](entt::entity src, entt::entity target)
    {
        const auto mob = World::registry->try_get<Mob>(target);

        const auto mob_tr = World::registry->try_get<Transform>(target);
        auto eff = World::registry->try_get<Effect>(target);
        eff->effects.push_back({new Transform(mob_tr->position + SDL_FPoint{0, -25}), hit});

        mob->call_backs.erase(u"2201004");
        // 冰冻效果,2秒
        auto call_back = [time = Window::dt_now + 2000](entt::entity ent)
        {
            const auto mob = World::registry->try_get<Mob>(ent);
            if (Window::dt_now <= time && mob->state != Mob::State::DIE && mob->state != Mob::State::REMOVE)
            {
                // 默认选择被攻击的第0帧
                auto sprw = mob->a[u"hit1"].asprw->sprites[0];
                mob->state = Mob::State::HIT;
                mob->index = u"hit1";
                mob->a[u"hit1"].anim_time = 0;
                mob->mod = SDL_Color{90, 148, 247};
                mob_fall(ent, Window::delta_time);
                return std::make_pair(false, false);
            }
            else
            {
                mob->mod = SDL_Color{255, 255, 255};
                return std::make_pair(true, true);
            }
        };
        mob->call_backs.emplace(u"2201004", call_back);
    };

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK | PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}