#include "PlayerSkill.h"
#include "Common.h"
#include "Commons/Commons.h"
#include "Systems/MobStateMachine.h"
#include "Components/Components.h"
#include "Entities/Entities.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Systems/Attack.h"

// 烈火箭
int skill_3111003(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 3111003);

    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    auto hit = ski->skiw->hits[0];
    auto mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    auto attackCount = 1;
    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 0);
    ski->atk.value().call_back = [mobCount](entt::entity src, entt::entity target, int full_damage)
    {
        auto ski = World::registry->try_get<Skill>(src);
        auto atk = &ski->atk.value();
        atk->damage = 20;
        atk->call_back = std::nullopt;

        auto time = 3000;
        auto target_position = World::registry->try_get<Transform>(target)->position;
        while (World::registry->valid(target) && atk->mobCount > 0)
        {
            const auto mob = World::registry->try_get<Mob>(target);
            const auto target_tr = World::registry->try_get<Transform>(target);

            // 执行攻击效果
            const SDL_FPoint hit_point = target_tr->position + mob->head(target_tr->flip);
            atk->mobCount--;
            attack_mob(atk, src, target, hit_point);
            mob->call_backs.erase(3111003);
            mob->call_backs.emplace(3111003, std::make_pair(flame_call_back, std::make_tuple(Window::dt_now + time, Window::dt_now + 500)));

            ski->hit_targets.insert(target);
            // 寻找下一个目标
            target = find_closest_attackable_mob(
                -1,
                target_position,
                ski->hit_targets,
                100.0f, // max_x_distance
                100.0f  // max_y_distance
            );
        }
    };

    World::registry->emplace_or_replace<AfterImage>(ent);

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}