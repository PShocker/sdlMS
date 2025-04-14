#include "PlayerSkill.h"
#include "Common.h"
#include "Commons/Commons.h"
#include "Systems/MobStateMachine.h"
#include "Components/Components.h"
#include "Entities/Entities.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Systems/Attack.h"

static std::pair<bool, bool> mob_call_back(entt::entity ent, std::any data)
{
    auto [atk, time] = std::any_cast<std::tuple<Attack, unsigned int>>(data);
    if (Window::dt_now >= time)
    {
        auto mob = World::registry->try_get<Mob>(ent);
        auto target_tr = World::registry->try_get<Transform>(ent);
        const SDL_FPoint hit_point = target_tr->position + mob->head(target_tr->flip);
        attack_mob(&atk, Player::ent, ent, hit_point);
        mob->call_backs.emplace(3101005, std::make_pair(dizzy_call_back, Window::dt_now + 5000));
        return std::make_pair(true, true);
    }
    return std::make_pair(false, true);
}

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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 3101005);

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
    ski->atk.value().call_back = [](entt::entity src, entt::entity target, int full_damage)
    {
        auto ski = World::registry->try_get<Skill>(src);
        auto atk = &ski->atk.value();
        atk->call_back = std::nullopt;
        atk->damage = 50;

        const auto *src_tr = World::registry->try_get<Transform>(src);
        atk->src_point = src_tr->position;

        auto target_mob = World::registry->try_get<Mob>(target);
        target_mob->call_backs.erase(3101005);
        target_mob->call_backs.emplace(3101005, std::make_pair(mob_call_back, std::make_tuple(ski->atk.value(), Window::dt_now)));

        auto target_position = World::registry->try_get<Transform>(target)->position;
        ski->hit_targets.insert(target);
        while (atk->mobCount > 0)
        {
            // 寻找下一个目标
            target = find_closest_attackable_mob(
                -1,
                target_position,
                ski->hit_targets,
                200.0f, // max_x_distance
                150.0f  // max_y_distance
            );
            if (target == entt::null)
            {
                break;
            }
            target_mob = World::registry->try_get<Mob>(target);
            target_mob->call_backs.erase(3101005);
            target_mob->call_backs.emplace(3101005, std::make_pair(mob_call_back, std::make_tuple(ski->atk.value(), Window::dt_now + 150)));
            ski->hit_targets.insert(target);
            atk->mobCount--;
        };
    };

    World::registry->emplace_or_replace<AfterImage>(ent);

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}