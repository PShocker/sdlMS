#include "PlayerSkill.h"
#include "Systems/Hit.h"
#include "Commons/Commons.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <numbers>

// 独立函数：寻找最近的可攻击怪物
entt::entity find_closest_attackable_mob(
    const SDL_FPoint &origin,
    const std::unordered_set<entt::entity> &hit_entities,
    float max_x_distance,
    float max_y_distance)
{
    entt::entity closest_mob = entt::null;
    float min_sq_dist = std::numeric_limits<float>::max();

    for (auto e : World::registry->view<Damage, Mob>())
    {
        if (hit_entities.contains(e))
            continue;

        const auto *mob = World::registry->try_get<Mob>(e);
        if (!mob || mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE)
            continue;

        const auto *tr = World::registry->try_get<Transform>(e);
        if (!tr)
            continue;

        // 快速空间过滤
        const float dx = std::abs(tr->position.x - origin.x);
        const float dy = std::abs(tr->position.y - origin.y);
        if (dx > max_x_distance || dy > max_y_distance)
            continue;

        // 计算平方距离
        const float sq_dist = squared_distance(tr->position, origin);
        if (sq_dist < min_sq_dist)
        {
            min_sq_dist = sq_dist;
            closest_mob = e;
        }
    }

    return closest_mob;
}

// 独立函数：生成链式特效
void generate_chain_effect(
    const SDL_FPoint &start,
    const SDL_FPoint &end,
    entt::entity target,
    Skill *ski)
{
    auto *eff = World::registry->try_get<Effect>(target);
    if (!eff)
        return;

    constexpr float segment_length = 50.0f;
    const float angle = calculate_angle(start, end);
    const float total_distance = distance(start, end);
    const int segments = static_cast<int>(total_distance / segment_length);

    const float dx_per_segment = (end.x - start.x) / segments;
    const float dy_per_segment = (end.y - start.y) / segments;

    static auto sprite_template = AnimatedSpriteWarp::load(ski->skiw->node->find_from_path(u"ball/0"));

    for (int i = 0; i < segments; ++i)
    {
        auto tr = new Transform(
            start.x + dx_per_segment * i,
            start.y + dy_per_segment * i);
        tr->rotation = angle;

        eff->effects.push_back({tr,
                                AnimatedSprite(sprite_template)}
                               // 复制预加载的模板
        );
    }
}

// 链环闪电技能（优化后）
int skill_2221006(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    if (!mv || !tr || !cha)
        return PlayerSkill::SkillResult::None;

    // 状态检查
    if (cha->state == Character::State::CLIMB)
    {
        return PlayerSkill::SkillResult::None;
    }
    else if (cha->state != Character::State::JUMP && mv->foo)
    {
        mv->hspeed = 0;
    }

    // 初始化技能
    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"2221006");
    const auto *skiw = ski->skiw;
    if (!skiw)
        return PlayerSkill::SkillResult::None;

    // 配置攻击参数
    auto level_node = skiw->level[ski->level];
    auto lt = SDL_FPoint{0, 0};
    auto rb = SDL_FPoint{0, 0};
    auto hit = ski->skiw->hits[0];
    auto attackCount = 1;
    const int mobCount = dynamic_cast<wz::Property<int> *>(level_node->get_child(u"mobCount"))->get();
    SoundWarp *souw = nullptr;
    if (ski->skiw->sounds.contains(u"Hit"))
    {
        souw = ski->skiw->sounds[u"Hit"];
    }
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 50);

    // 回调函数（优化后）
    ski->call_back = [](entt::entity src)
    {
        auto *ski = World::registry->try_get<Skill>(src);
        auto *atk = ski ? &ski->atk.value() : nullptr;
        if (!atk || atk->mobCount <= 0)
            return;

        const auto *src_tr = World::registry->try_get<Transform>(src);
        if (!src_tr)
            return;

        std::unordered_set<entt::entity> hit_targets;
        const SDL_FPoint origin = src_tr->position;

        // 第一目标搜索
        entt::entity target = find_closest_attackable_mob(
            origin,
            hit_targets,
            500.0f, // max_x_distance
            90.0f   // max_y_distance
        );

        SDL_FPoint src_point = src_tr->position +
                               (src_tr->flip ? SDL_FPoint{30, -35} : SDL_FPoint{-10, -35});
        while (World::registry->valid(target) && atk->mobCount > 0)
        {
            const auto mob = World::registry->try_get<Mob>(target);
            const auto target_tr = World::registry->try_get<Transform>(target);
            if (!mob || !target_tr)
                break;

            // 执行攻击效果
            const SDL_FPoint hit_point = target_tr->position + mob->head();
            hit_effect(atk, mob->head(), target, 0, hit_point);

            // 生成特效
            generate_chain_effect(src_point, hit_point, target, ski);
            src_point = hit_point;

            hit_targets.insert(target);
            atk->mobCount--;

            // 寻找下一个目标
            target = find_closest_attackable_mob(
                target_tr->position,
                hit_targets,
                200.0f, // max_x_distance
                150.0f  // max_y_distance
            );
        }
    };

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}