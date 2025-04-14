#include "PlayerSkill.h"
#include "Common.h"
#include "Commons/Commons.h"
#include "Systems/Attack.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <numbers>

// 独立函数：生成链式特效
static void generate_chain_effect(
    const SDL_FPoint &start,
    const SDL_FPoint &end,
    entt::entity target)
{
    auto eff = World::registry->try_get<Effect>(target);

    const float segment_length = 50.0f;
    float angle = calculate_angle(start, end);
    float total_distance = distance(start, end);
    float segments = (total_distance / segment_length);

    float dx_per_segment = (end.x - start.x) / segments;
    float dy_per_segment = (end.y - start.y) / segments;

    auto sprite_template = AnimatedSprite::Wrap::load(Skill::Wrap::load(2221006)->node->find_from_path(u"ball/0"));

    for (float i = 0; i < segments; ++i)
    {
        Transform tr(
            start.x + dx_per_segment * i,
            start.y + dy_per_segment * i);
        tr.rotation = angle;

        eff->effects.emplace(0, Effect::Wrap{tr,
                                             AnimatedSprite(sprite_template)});
    }
}

static std::pair<bool, bool> mob_call_back(entt::entity ent, std::any data)
{
    auto [src, atk, hit_targets, src_point, mobCount, time] = std::any_cast<std::tuple<entt::entity, Attack, std::unordered_set<entt::entity>, SDL_FPoint, int, unsigned int>>(data);
    if (Window::dt_now >= time)
    {
        auto position = World::registry->try_get<Transform>(ent)->position;
        auto target = find_closest_attackable_mob(
            -1,
            position,
            hit_targets,
            300.0f, // max_x_distance
            200.0f  // max_y_distance
        );
        if (target != entt::null && mobCount > 0)
        {
            const auto target_tr = World::registry->try_get<Transform>(target);
            const auto target_mob = World::registry->try_get<Mob>(target);
            const SDL_FPoint hit_point = target_tr->position + target_mob->head(target_tr->flip) +
                                         SDL_FPoint{0, (float)target_mob->a[target_mob->index].asprw->sprites[target_mob->a[target_mob->index].anim_index]->texture->h / 2};
            generate_chain_effect(src_point, hit_point, target);
            hit_targets.insert(target);
            mobCount--;
            attack_mob(&atk, src, target, hit_point);
            target_mob->call_backs.emplace(2221006, std::make_pair(mob_call_back, std::make_tuple(src, atk, hit_targets, hit_point, mobCount, Window::dt_now + 70)));
        }
        return std::make_pair(true, true);
    }
    return std::make_pair(false, true);
}

// 链环闪电技能（优化后）
int skill_2221006(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
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
    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 2221006);
    auto skiw = ski->skiw;

    // 配置攻击参数
    auto level_node = skiw->level[ski->level];
    auto lt = SDL_FPoint{0, 0};
    auto rb = SDL_FPoint{0, 0};
    auto hit = skiw->hits[0];
    auto attackCount = 1;
    auto mobCount = 0;
    Sound::Wrap *souw = skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 50);

    // 回调函数（优化后）
    ski->call_back = [mobCount = dynamic_cast<wz::Property<int> *>(level_node->get_child(u"mobCount"))->get()](entt::entity src, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 1))
        {
            return;
        }
        auto ski = World::registry->try_get<Skill>(src);
        auto atk = &ski->atk.value();

        const auto *src_tr = World::registry->try_get<Transform>(src);
        atk->src_point = src_tr->position;

        // 第一目标搜索
        entt::entity target = find_closest_attackable_mob(
            src_tr->flip,
            src_tr->position,
            ski->hit_targets,
            500.0f, // max_x_distance
            90.0f   // max_y_distance
        );

        if (target != entt::null)
        {
            SDL_FPoint src_point = src_tr->position +
                                   (src_tr->flip ? SDL_FPoint{30, -25} : SDL_FPoint{-30, -25});
            auto target_mob = World::registry->try_get<Mob>(target);
            target_mob->call_backs.emplace(2221006, std::make_pair(mob_call_back, std::make_tuple(src, ski->atk.value(), ski->hit_targets, src_point, mobCount, Window::dt_now)));
        }
    };

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}