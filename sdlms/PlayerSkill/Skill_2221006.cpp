#include "PlayerSkill.h"
#include "Systems/Hit.h"
#include "Commons/Commons.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <numbers>

// 链环闪电
int skill_2221006(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"2221006");
    auto lt = SDL_FPoint{0, 0};
    auto rb = SDL_FPoint{0, 0};
    auto hit = ski->skiw->hits[0];
    auto attackCount = 1;
    auto node = ski->skiw->level[ski->level];
    auto mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    SoundWarp *souw = nullptr;
    if (ski->skiw->sounds.contains(u"Hit"))
    {
        souw = ski->skiw->sounds[u"Hit"];
    }
    ski->atkw = AttackWarp(lt, rb, hit, mobCount, attackCount, souw, 50);

    auto call_back = [](entt::entity ent)
    {
        Skill *ski = World::registry->try_get<Skill>(ent);
        const auto effect_func = [&ski](SDL_FPoint m, SDL_FPoint n, entt::entity e)
        {
            auto l = m.x < n.x ? m : n;
            auto r = m.x > n.x ? m : n;
            double dy = r.y - l.y;
            double dx = r.x - l.x;
            float k = 0;
            float angle;
            // 避免除以零
            if (dx == 0)
            {
                angle = (dy > 0) ? 90.0 : 270.0; // 垂直的情况
            }
            else
            {
                k = dy / dx;
                // 计算夹角（弧度）
                double angle_rad = std::atan(k);
                // 转换为度
                angle = angle_rad * (180.0 / std::numbers::pi);
                if (angle < 0)
                {
                    angle += 360.0;
                }
            }
            // 绘制攻击特效线
            auto eff = World::registry->try_get<Effect>(e);
            auto dis = distance(l, r);
            const auto length = 50;
            double a = length / sqrt(1 + k * k);
            double b = k * a;
            for (int i = 0; i < dis / length; i++)
            {
                auto tr = new Transform(l.x + i * a, l.y + i * b);
                tr->rotation = angle;
                eff->effects.push_back({tr, AnimatedSprite(AnimatedSpriteWarp::load(ski->skiw->node->find_from_path(u"ball/0")))});
            }
        };
        std::unordered_set<entt::entity> hit;
        const auto find_min_mob = [&hit](entt::entity ent, bool player)
        {
            int dis = INT32_MAX;
            entt::entity target = entt::null;
            for (auto e : World::registry->view<Damage, Mob>())
            {
                if (!hit.contains(e))
                {
                    auto mob = World::registry->try_get<Mob>(e);
                    if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
                    {
                        auto m_tr = World::registry->try_get<Transform>(e);
                        if (player)
                        {
                            auto p_tr = World::registry->try_get<Transform>(ent);
                            if (std::abs(p_tr->position.y - m_tr->position.y) <= 90)
                            {
                                if ((p_tr->flip == 1 && p_tr->position.x <= m_tr->position.x && (m_tr->position.x - p_tr->position.x) <= 500) ||
                                    (p_tr->flip == 0 && p_tr->position.x >= m_tr->position.x && (p_tr->position.x - m_tr->position.x) <= 500))
                                {
                                    if (auto d = distance(m_tr->position, p_tr->position); d < dis)
                                    {
                                        target = e;
                                        dis = d;
                                    }
                                }
                            }
                        }
                        else
                        {
                            auto t_tr = World::registry->try_get<Transform>(ent);
                            if (std::abs(t_tr->position.y - m_tr->position.y) <= 90 &&
                                std::abs(t_tr->position.x - m_tr->position.x) <= 200)
                            {
                                if (auto d = distance(m_tr->position, t_tr->position); d < dis)
                                {
                                    target = e;
                                    dis = d;
                                }
                            }
                        }
                    }
                }
            }
            return target;
        };

        auto atkw = &ski->atkw.value();

        auto target = find_min_mob(ent, true);
        if (World::registry->valid(target))
        {
            auto mob = World::registry->try_get<Mob>(target);
            auto t_tr = World::registry->try_get<Transform>(target);
            auto p_tr = World::registry->try_get<Transform>(ent);
            atkw->p = p_tr->position;
            hit_effect(atkw, mob->head(), target, 0, mob->head() + t_tr->position);
            atkw->mobCount -= 1;
            if (p_tr->flip == 1)
            {
                effect_func(p_tr->position + SDL_FPoint{30, -35}, t_tr->position + mob->head(), target);
            }
            else
            {
                effect_func(p_tr->position + SDL_FPoint{-10, -35}, t_tr->position + mob->head(), target);
            }
            hit.insert(target);
            while (true)
            {
                // last是前一个怪物
                auto last = target;
                if (atkw->mobCount > 0)
                {
                    target = find_min_mob(last, false);
                    if (World::registry->valid(target))
                    {
                        auto mob = World::registry->try_get<Mob>(target);
                        auto l_tr = World::registry->try_get<Transform>(last);
                        t_tr = World::registry->try_get<Transform>(target);
                        atkw->p = l_tr->position;
                        hit_effect(atkw, mob->head(), target, 0, mob->head() + t_tr->position);
                        atkw->mobCount -= 1;
                        effect_func(l_tr->position + World::registry->try_get<Mob>(last)->head(), t_tr->position + mob->head(), target);
                        hit.insert(target);
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }
    };

    ski->call_back = call_back;

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU | PlayerSkill::SkillResult::ACT | PlayerSkill::SkillResult::ALERT;
}