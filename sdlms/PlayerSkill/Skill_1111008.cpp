#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Systems/Camera.h"
#include "Common.h"
#include "Systems/MobStateMachine.h"

// 虎咆哮
int skill_1111008(entt::entity ent)
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
    // 虎咆哮需要震动摄像机
    camera_init_shake(12, 120, 1060);

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 1111008);

    auto eff = World::registry->try_get<Effect>(ent);
    eff->effects.emplace(-1111008, Effect::Wrap{std::nullopt, AnimatedSprite(ski->skiw->effects[0])});

    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    auto hit = ski->skiw->hits[0];
    auto mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    auto attackCount = 1;
    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 90);
    ski->atk.value().call_back = [](entt::entity src, entt::entity target, int full_damage)
    {
        auto ski = World::registry->try_get<Skill>(src);

        // 晕眩效果,5秒
        const auto mob = World::registry->try_get<Mob>(target);

        mob->call_backs.erase(1111008);
        mob->call_backs.emplace(1111008, std::make_pair(dizzy_call_back, Window::dt_now + 5000));
    };

    return PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}