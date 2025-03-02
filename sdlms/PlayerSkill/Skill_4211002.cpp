#include "PlayerSkill.h"
#include "Commons/Commons.h"
#include "Systems/Move.h"
#include "Systems/Hit.h"
#include "Systems/Collision.h"
#include "Systems/Attack.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 落叶斩
int skill_4211002(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"4211002");

    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    // 这个技能需要加强
    auto attackCount = 4;
    auto mobCount = 12;

    SoundWarp *souw = nullptr;
    if (ski->skiw->sounds.contains(u"Hit"))
    {
        souw = ski->skiw->sounds[u"Hit"];
    }
    ski->atkw = AttackWarp(lt, rb, nullptr, mobCount, attackCount, souw, 50);

    auto call_back = [](entt::entity ent)
    {
        Skill *ski = World::registry->try_get<Skill>(ent);
        auto atkw = &ski->atkw.value();
        attack_mob(atkw, ent);

        auto o_tr = World::registry->try_get<Transform>(ent);
        auto o_mv = World::registry->try_get<Move>(ent);
        auto eff = World::registry->try_get<Effect>(ent);
        if (o_tr->flip == 1)
        {
            o_mv->hspeed = 125;
            o_mv->hforce = 1400;
            eff->effects.push_back({new Transform(o_tr->position + SDL_FPoint{100, -35}, 0, o_tr->flip), AnimatedSprite(ski->skiw->hits[0])});
        }
        else
        {
            o_mv->hspeed = -125;
            o_mv->hforce = -1400;
            eff->effects.push_back({new Transform(o_tr->position + SDL_FPoint{-100, -35}, 0, o_tr->flip), AnimatedSprite(ski->skiw->hits[0])});
        }
        eff->effects.push_back({new Transform(o_tr->position, 0, o_tr->flip), AnimatedSprite(Effect::load(u"BasicEff.img/Assaulter/remain"))});
        if (o_mv->foo)
        {
            // 地面上
            move_move(o_mv, o_tr, 800, 1.5, false);
            o_mv->hspeed = 0;
            o_mv->hforce = 0;
        }
        else
        {
            // 空中
            o_mv->vspeed = 0;
            move_fall(o_mv, o_tr, 1.5, o_tr->z % LAYER_Z);
            o_mv->hspeed = 0;
        }
        // 添加effect
        eff->effects.push_back({nullptr, AnimatedSprite(Effect::load(u"BasicEff.img/Assaulter/effect"))});
        eff->effects.push_back({nullptr, AnimatedSprite(Effect::load(u"BasicEff.img/Assaulter/effect0"))});
    };
    ski->call_back = call_back;

    return PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}