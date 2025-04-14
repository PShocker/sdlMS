#include "PlayerSkill.h"
#include "Commons/Commons.h"
#include "Systems/Move.h"
#include "Systems/Collision.h"
#include "Systems/Attack.h"
#include "Resources/Wz.h"
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 4211002);

    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    auto attackCount = 4;
    auto mobCount = 12;

    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, nullptr, mobCount, attackCount, souw, 100);

    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        Skill *ski = World::registry->try_get<Skill>(ent);
        if (!(action_time == 0 && action_frame == 1))
        {
            return;
        }

        auto atk = &ski->atk.value();
        auto owner_tr = World::registry->try_get<Transform>(ent);
        atk->hit = owner_tr->flip == 1 ? AnimatedSprite::Wrap::load(Wz::Skill->get_root()->find_from_path(u"400.img/skill/4001334/hit/1"))
                                       : AnimatedSprite::Wrap::load(Wz::Skill->get_root()->find_from_path(u"400.img/skill/4001334/hit/0"));
        attack_mob(atk, ent);

        auto owner_mv = World::registry->try_get<Move>(ent);
        auto eff = World::registry->try_get<Effect>(ent);
        if (owner_tr->flip == 1)
        {
            owner_mv->hspeed = 125;
            owner_mv->hforce = 1400;
            eff->effects.emplace(4211002,
                                 Effect::Wrap{Transform(owner_tr->position + SDL_FPoint{100, -35}, 0, owner_tr->flip), AnimatedSprite(ski->skiw->hits[0])});
        }
        else
        {
            owner_mv->hspeed = -125;
            owner_mv->hforce = -1400;
            eff->effects.emplace(4211002,
                                 Effect::Wrap{Transform(owner_tr->position + SDL_FPoint{-100, -35}, 0, owner_tr->flip), AnimatedSprite(ski->skiw->hits[0])});
        }
        eff->effects.emplace(4211002,
                             Effect::Wrap{Transform(owner_tr->position, 0, owner_tr->flip), AnimatedSprite(Effect::load(u"BasicEff.img/Assaulter/remain"))});
        auto hspeed_min = owner_mv->hspeed_min;
        auto hspeed_max = owner_mv->hspeed_max;
        owner_mv->hspeed_min = -125;
        owner_mv->hspeed_max = 125;
        if (owner_mv->foo)
        {
            // 地面上
            move_move(owner_mv, owner_tr, 800, 1.5, false);
            owner_mv->hspeed = 0;
            owner_mv->hforce = 0;
        }
        else
        {
            // 空中
            owner_mv->vspeed = 0;
            move_fall(owner_mv, owner_tr, 1.5, owner_tr->z % LAYER_Z);
            owner_mv->hspeed = 0;
        }
        owner_mv->hspeed_min = hspeed_min;
        owner_mv->hspeed_max = hspeed_max;
        // 添加effect
        eff->effects.emplace(4211002, Effect::Wrap{std::nullopt, AnimatedSprite(Effect::load(u"BasicEff.img/Assaulter/effect"))});
        eff->effects.emplace(4211002, Effect::Wrap{std::nullopt, AnimatedSprite(Effect::load(u"BasicEff.img/Assaulter/effect0"))});
    };

    return PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}