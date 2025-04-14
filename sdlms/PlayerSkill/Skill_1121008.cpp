#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 轻舞飞扬
int skill_1121008(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 1121008);

    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    auto attackCount = 1;
    auto mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    auto hit = ski->skiw->hits[0];
    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];

    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 20);
    ski->atk.value().call_back = [](entt::entity src, entt::entity target, int full_damage)
    {
        auto ski = World::registry->try_get<Skill>(src);
        ski->hit = false;
    };

    ski->call_back = [&atk = ski->atk, mobCount](entt::entity ent, int action_frame, int action_time)
    {
        atk->mobCount = mobCount;
    };

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK | PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}