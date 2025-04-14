#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 回旋斩
int skill_4201005(entt::entity ent)
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

    auto weaponWrap = World::registry->try_get<WeaponWrap>(ent);

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 4201005);

    auto lt = SDL_FPoint{-130, -32};
    auto rb = SDL_FPoint{-40, -11};
    auto node = ski->skiw->level[ski->level];
    auto attackCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"attackCount"))->get();
    auto mobCount = 1;
    auto hit = ski->skiw->hits[0];
    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];

    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 20);
    ski->atk.value().call_back = [attackCount](entt::entity src, entt::entity target, int full_damage)
    {
        auto ski = World::registry->try_get<Skill>(src);
        auto eff = World::registry->try_get<Effect>(target);
        for (int i = 0; i < attackCount - 1; ++i)
        {
            eff->effects.emplace(4201005, Effect::Wrap{std::nullopt,
                                                       AnimatedSprite(ski->skiw->hits[0]), Window::dt_now + i * 128});
        }
    };

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK | PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}