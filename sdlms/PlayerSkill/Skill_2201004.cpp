#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Systems/MobStateMachine.h"
#include "Commons/Commons.h"
#include "Common.h"

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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 2201004);

    auto node = ski->skiw->level[ski->level];
    auto lt = SDL_FPoint{-180, -32};
    auto rb = SDL_FPoint{-40, -11};
    auto attackCount = 1;
    auto mobCount = 1;
    auto hit = ski->skiw->hits[0];
    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];

    ski->atk = Attack(lt, rb, nullptr, mobCount, attackCount, souw, 30);
    ski->atk.value().call_back = [hit](entt::entity src, entt::entity target, int full_damage)
    {
        const auto mob = World::registry->try_get<Mob>(target);

        const auto mob_tr = World::registry->try_get<Transform>(target);
        auto eff = World::registry->try_get<Effect>(target);
        eff->effects.emplace(2201004, Effect::Wrap{Transform(SDL_FPoint{0, -25}), hit, Window::dt_now, true});
        mob->call_backs.erase(2201004);
        mob->call_backs.emplace(2201004, std::make_pair(frozen_call_back, Window::dt_now + 5000));
    };

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK | PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}