#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include "Common.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"
#include <SDL3/SDL.h>

// 火焰箭
int skill_2101004(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 2101004);

    auto lt = SDL_FPoint{0, 0};
    auto rb = SDL_FPoint{0, 0};
    auto hit = ski->skiw->hits[0];
    auto mobCount = 1;
    auto attackCount = 1;
    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 10);

    ski->atk.value().call_back = [](entt::entity src,
                                    entt::entity target,
                                    int full_damage)
    {
        const auto mob = World::registry->try_get<Mob>(target);
        unsigned int time = 6000;

        mob->call_backs.erase(2101004);
        mob->call_backs.emplace(2101004, std::make_pair(flame_call_back, std::make_tuple(Window::dt_now + time, Window::dt_now + 500)));
    };

    Skill::cooldowns[2101004] = Window::dt_now + 500;

    return PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ATK |
           PlayerSkill::SkillResult::ALERT;
}