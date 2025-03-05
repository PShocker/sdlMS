#include "PlayerSkill.h"
#include "Common.h"
#include "Commons/Commons.h"
#include "Systems/Hit.h"
#include "Components/Components.h"
#include "Entities/Entities.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

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
    Attack atk(lt, rb, hit, mobCount, attackCount, souw);

    atk.call_back = [](entt::entity src, entt::entity target)
    {
        auto *ski = World::registry->try_get<Skill>(src);
        auto *atk = &ski->atk.value();
        atk->call_back = std::nullopt;
        auto target_position = World::registry->try_get<Transform>(target)->position;

        std::unordered_set<entt::entity> hit_targets;
        do
        {
            const auto mob = World::registry->try_get<Mob>(target);
            const auto target_tr = World::registry->try_get<Transform>(target);

            // 执行攻击效果
            const SDL_FPoint hit_point = target_tr->position + mob->head();
            hit_effect(atk, mob->head(), src, target, 0, hit_point);

            hit_targets.insert(target);
            atk->mobCount--;

            // 寻找下一个目标
            target = find_closest_attackable_mob(
                -1,
                target_position,
                hit_targets,
                200.0f, // max_x_distance
                150.0f  // max_y_distance
            );
        } while (World::registry->valid(target) && atk->mobCount > 0);
        return false;
    };
    ski->ball = 1;
    ski->atk = atk;

    World::registry->emplace_or_replace<AfterImage>(ent);

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}