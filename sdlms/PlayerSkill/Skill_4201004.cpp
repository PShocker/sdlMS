#include "PlayerSkill.h"
#include "Commons/Commons.h"
#include "Components/Components.h"
#include "Entities/Entities.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 神通术
int skill_4201004(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 4201004);

    auto lt = SDL_FPoint{-110, -32};
    auto rb = SDL_FPoint{-40, -11};
    auto hit = nullptr;
    auto mobCount = 1;
    auto attackCount = 1;
    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 10);

    ski->atk.value().call_back = [](entt::entity src, entt::entity target, int full_damage)
    {
        auto tr = World::registry->try_get<Transform>(target);
        std::vector<std::pair<std::u16string, unsigned int>> drops = {{u"09000000", (unsigned int)std::rand() % 200}};
        load_drops(drops, tr->position.x, tr->position.y, tr->z / LAYER_Z);
        Sound::push(Sound(u"Game.img/DropItem", 200));
        World::zindex = true;
    };

    auto weaponWrap = World::registry->try_get<WeaponWrap>(ent);
    if (WeaponWrap::if_long_range_weapon(weaponWrap->attack))
    {
        auto action = weaponWrap->degen_stances[weaponWrap->attack][std::rand() % weaponWrap->degen_stances[weaponWrap->attack].size()];
        ski->skiw->action_str = Character::type_map2.at(action);
    }
    else
    {
        ski->skiw->action_str = u"";
    }
    World::registry->emplace_or_replace<AfterImage>(ent);

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ATK |
           PlayerSkill::SkillResult::ALERT;
}