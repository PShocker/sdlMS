#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include "Resources/Wz.h"
#include <SDL3/SDL.h>

// 双飞镖
int skill_4001344(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto weaponinfo = World::registry->try_get<WeaponInfo>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    // 通用攻击技能
    if (state == Character::State::CLIMB || weaponinfo->attack != WeaponInfo::CLAW)
    {
        return PlayerSkill::SkillResult::None;
    }
    else if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }

    auto effect = Wz::Skill->get_root()->find_from_path(u"400.img/skill/4001344/CharLevel/20/effect");
    auto eff = World::registry->try_get<Effect>(ent);
    eff->effects.push_back({nullptr, AnimatedSprite(AnimatedSpriteWarp::load(effect))});

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"4001344");

    auto lt = SDL_FPoint{0, 0};
    auto rb = SDL_FPoint{0, 0};
    auto hit = ski->skiw->hits[0];
    auto mobCount = 1;
    auto attackCount = 1;
    SoundWarp *souw = nullptr;
    if (ski->skiw->sounds.contains(u"Hit"))
    {
        souw = ski->skiw->sounds[u"Hit"];
    }
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 50);
    ski->ball = 2;

    SkillWarp::cooldowns[u"4001344"] = 500;

    return PlayerSkill::SkillResult::SOU | PlayerSkill::SkillResult::ACT | PlayerSkill::SkillResult::ALERT;
}