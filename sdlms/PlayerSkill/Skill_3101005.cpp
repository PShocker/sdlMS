#include "PlayerSkill.h"
#include "Common.h"
#include "Commons/Commons.h"
#include "Systems/MobStateMachine.h"
#include "Components/Components.h"
#include "Entities/Entities.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Systems/Attack.h"

// 爆炸箭
int skill_3101005(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"3101005");

    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    auto hit = ski->skiw->hits[0];
    auto mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    auto attackCount = 1;
    SoundWarp *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 0);
    ski->atk.value().call_back = [](entt::entity src, entt::entity target, int full_damage)
    {
        auto ski = World::registry->try_get<Skill>(src);
        auto atk = &ski->atk.value();
        atk->damage = 50;
        atk->call_back = std::nullopt;
        // 晕眩效果,3秒
        const auto call_back = [asprw = AnimatedSpriteWarp::load(ski->skiw->node->find_from_path(u"mob")),
                                time = Window::dt_now + 3000](entt::entity ent)
        {
            const auto mob = World::registry->try_get<Mob>(ent);

            if (Window::dt_now <= time && mob->state != Mob::State::DIE && mob->state != Mob::State::REMOVE)
            {
                // 晕眩特效
                push_mob_special_effect(ent, Effect::Dizzy, asprw);
                mob_fall(ent, Window::delta_time);
                return std::make_pair(false, false);
            }
            else
            {
                return std::make_pair(true, true);
            }
        };
        auto target_position = World::registry->try_get<Transform>(target)->position;
        while (World::registry->valid(target) && atk->mobCount > 0)
        {
            const auto mob = World::registry->try_get<Mob>(target);
            const auto target_tr = World::registry->try_get<Transform>(target);

            mob->call_backs.erase(u"3101005");

            // 执行攻击效果
            const SDL_FPoint hit_point = target_tr->position + mob->head(target_tr->flip);
            atk->mobCount--;
            attack_mob(atk, src, target, hit_point);
            mob->call_backs.emplace(u"3101005", call_back);
            ski->hit_targets.insert(target);
            // 寻找下一个目标
            target = find_closest_attackable_mob(
                -1,
                target_position,
                ski->hit_targets,
                200.0f, // max_x_distance
                150.0f  // max_y_distance
            );
        };
    };

    World::registry->emplace_or_replace<AfterImage>(ent);

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}