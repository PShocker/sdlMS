#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 双飞镖
int skill_4001344()
{
    auto mv = World::registry->try_get<Move>(Player::ent);
    auto cha = World::registry->try_get<Character>(Player::ent);
    auto state = cha->state;

    // 通用攻击技能
    if (state == Character::State::CLIMB)
    {
        return PlayerSkill::SkillResult::None;
    }
    else if (state != Character::State::JUMP)
    {
        mv->hspeed = 0;
    }

    auto ski = &World::registry->emplace_or_replace<Skill>(Player::ent, u"4001344");

    auto lt = SDL_FPoint{0, 0};
    auto rb = SDL_FPoint{0, 0};
    auto hit = ski->ski->hits[0];
    auto mobCount = 1;
    auto attackCount = 1;
    SoundWarp *souw = nullptr;
    if (ski->ski->sounds.contains(u"Hit"))
    {
        souw = ski->ski->sounds[u"Hit"];
    }
    ski->atkw = AttackWarp(lt, rb, hit, mobCount, attackCount, souw);
    ski->ball = 2;

    SkillWarp::cooldowns[u"4001344"] = 500;

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT | PlayerSkill::SkillResult::ALERT;
}