#include "Animate.h"
#include "Hit.h"
#include "Ball.h"
#include "Core/Core.h"
#include "Entities/Entities.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"
#include <SDL3/SDL.h>
#include "entt/entt.hpp"
#include <cmath>
#include <numbers>
#include <ranges>

void animate_run()
{
    auto view = World::registry->view<Animated>();

    for (auto ent : World::registry->view<Animated, AnimatedSprite>())
    {
        auto aspr = World::registry->try_get<AnimatedSprite>(ent);
        animate_sprite(aspr);
    }
    for (auto ent : World::registry->view<Animated, Portal>())
    {
        auto por = World::registry->try_get<Portal>(ent);
        animate_portal(por);
    }
    for (auto ent : World::registry->view<Animated, Npc>())
    {
        auto npc = World::registry->try_get<Npc>(ent);
        animate_npc(npc);
        if (auto eff = World::registry->try_get<Effect>(ent))
        {
            animate_effect(eff);
        }
        if (auto dam = World::registry->try_get<Damage>(ent))
        {
            animate_damage(dam);
        }
    }
    for (auto ent : World::registry->view<Animated, Mob>())
    {
        auto mob = World::registry->try_get<Mob>(ent);
        animate_mob(mob, ent);
        if (auto eff = World::registry->try_get<Effect>(ent))
        {
            animate_effect(eff);
        }
        if (auto dam = World::registry->try_get<Damage>(ent))
        {
            animate_damage(dam);
        }
    }
    for (auto ent : World::registry->view<Animated, Character>())
    {
        auto cha = World::registry->try_get<Character>(ent);
        animate_character(cha, ent);
        if (auto aft = World::registry->try_get<AfterImage>(ent))
        {
            if (aft->animated == false)
            {
                auto cha = World::registry->try_get<Character>(ent);
                animate_afterimage(aft, cha, ent);
            }
        }
        if (auto eff = World::registry->try_get<Effect>(ent))
        {
            animate_effect(eff);
        }
        if (auto dam = World::registry->try_get<Damage>(ent))
        {
            animate_damage(dam);
        }
        if (auto tomb = World::registry->try_get<Tomb>(ent))
        {
            auto tr = World::registry->try_get<Transform>(ent);
            animate_tomb(tomb, tr);
        }
        animate_face(cha);
    }
    for (auto ent : World::registry->view<Animated, Drop>())
    {
        auto dro = World::registry->try_get<Drop>(ent);
        animate_drop(dro, World::registry->try_get<Transform>(ent));
    }
}

bool animate_sprite(AnimatedSprite *a)
{
    bool r = true;
    if (a->animate)
    {
        auto delta_time = Window::delta_time;
        a->anim_time += delta_time;
        if (a->anim_time >= a->aspr->sprites[a->anim_index]->delay)
        {
            if (a->anim_index + a->anim_step == a->anim_size || a->anim_index + a->anim_step < 0)
            {
                if (a->aspr->z)
                {
                    if (a->anim_step == 1)
                    {
                        a->anim_index -= 1;
                        a->anim_step = -1;
                    }
                    else
                    {
                        a->anim_index += 1;
                        a->anim_step = 1;
                    }
                }
                else
                {
                    a->anim_index = 0;
                }
                a->anim_time = 0;
                r = false;
            }
            else
            {
                a->anim_index += a->anim_step;
                a->anim_time = 0;
            }
        }
        // 透明度处理
        auto a0 = a->aspr->sprites[a->anim_index]->a0;
        auto a1 = a->aspr->sprites[a->anim_index]->a1;
        a->alpha = a0;
        if (a0 != a1)
        {
            if (a0 <= a1)
            {
                a->alpha = (float)a0 + (float)(a1 - a0) / (float)a->aspr->sprites[a->anim_index]->delay * (float)a->anim_time;
            }
            else
            {
                a->alpha = (float)a0 - (float)(a0 - a1) / (float)a->aspr->sprites[a->anim_index]->delay * (float)a->anim_time;
            }
        }
    }
    return r;
}

void animate_character(Character *cha, entt::entity ent)
{
    if (cha->animate)
    {
        auto delta_time = Window::delta_time;
        cha->action_time += delta_time;
        if (Character::type_map.contains(cha->action_str))
        {
            // 基础动作
            auto delay = cha->stance_delays[cha->action][cha->action_index];
            if (auto ski = World::registry->try_get<Skill>(ent))
            {
                // 不包含特殊动作的技能动作,此时攻击帧需要从AfterImage获取
                auto action = cha->action;
                auto weaponinfo = World::registry->try_get<WeaponInfo>(ent);
                auto afterImage_index = AfterImage::afterImage_index(weaponinfo->reqLevel);
                auto &info = AfterImage::afterimages[weaponinfo->afterImage][afterImage_index][action];
                uint8_t index = info.index;
                if (cha->action_index == index && ski->hit == false)
                {
                    if (ski->attack == true)
                    {
                        auto atk = World::registry->try_get<Attack>(ent);
                        auto atkw = ski->atkw.value();
                        atkw.p = &World::registry->try_get<Transform>(Player::ent)->position;
                        atkw.damage = 50;
                        atk->atks.push_back(atkw);
                    }
                    else if (ski->ball > 0)
                    {
                        if (ski->ski->ball != nullptr)
                        {
                            for (auto e : load_ball(ski->ski->ball, ent, ski->ball))
                            {
                                auto &s = World::registry->emplace<Skill>(e);
                                s.atkw = ski->atkw;
                            }
                        }
                        else
                        {
                            entt::entity target = entt::null;
                            std::vector<entt::entity> ents = load_ball(ski->ball);
                            for (auto e : ents)
                            {
                                auto ball = World::registry->try_get<Ball>(e);
                                // 让ball聚集到一个怪物上
                                if ((target == entt::null || !World::registry->valid(target)))
                                {
                                    target = ball_fall(e, ball);
                                }
                                auto &s = World::registry->emplace<Skill>(e);
                                s.atkw = ski->atkw;
                            }
                            for (auto e : ents)
                            {
                                auto ball = World::registry->try_get<Ball>(e);
                                ball->target = target;

                                auto tr = World::registry->try_get<Transform>(e);
                                auto position = tr->position;
                                ball->p = position;
                            }
                        }
                    }
                    ski->hit = true;
                }
            }
            if (cha->action_time >= delay)
            {
                if (cha->action_index == cha->stance_delays[cha->action].size() - 1)
                {
                    cha->action_index = 0;
                    cha->animated = true;
                }
                else
                {
                    cha->action_index += 1;
                }
                cha->action_time = 0;
            }
        }
        else
        {
            // 技能动作
            int delay = cha->body_actions[cha->action_str][cha->action_frame].delay;
            if (delay > 0)
            {
                // 判断攻击帧
                if (auto ski = World::registry->try_get<Skill>(ent))
                {
                    if (ski->hit == false)
                    {
                        if (ski->attack == true)
                        {
                            auto atk = World::registry->try_get<Attack>(ent);
                            auto atkw = ski->atkw.value();
                            atkw.p = &World::registry->try_get<Transform>(Player::ent)->position;
                            atkw.damage = 50;
                            atk->atks.push_back(atkw);
                        }
                        else if (ski->ball > 0)
                        {
                            if (ski->ski->ball != nullptr)
                            {
                                for (auto e : load_ball(ski->ski->ball, ent, ski->ball))
                                {
                                    auto &s = World::registry->emplace<Skill>(e);
                                    s.atkw = ski->atkw;
                                }
                            }
                            else
                            {
                                entt::entity target = entt::null;
                                std::vector<entt::entity> ents = load_ball(ski->ball);
                                for (auto e : ents)
                                {
                                    auto ball = World::registry->try_get<Ball>(e);
                                    // 让ball聚集到一个怪物上
                                    if ((target == entt::null || !World::registry->valid(target)))
                                    {
                                        target = ball_fall(e, ball);
                                    }
                                    auto &s = World::registry->emplace<Skill>(e);
                                    s.atkw = ski->atkw;
                                }
                                for (auto e : ents)
                                {
                                    auto ball = World::registry->try_get<Ball>(e);
                                    ball->target = target;

                                    auto tr = World::registry->try_get<Transform>(e);
                                    auto position = tr->position;
                                    ball->p = position;
                                }
                            }
                        }
                        ski->hit = true;
                    }
                }
            }
            if (cha->action_time >= std::abs(delay))
            {
                if (cha->action_frame == cha->body_actions[cha->action_str].size() - 1)
                {
                    cha->action_index = 0;
                    cha->animated = true;
                }
                else
                {
                    cha->action = cha->body_actions[cha->action_str][cha->action_frame].type;
                    cha->action_index = cha->body_actions[cha->action_str][cha->action_frame].frame;
                    cha->action_frame += 1;
                }
                cha->action_time = 0;
            }
            else
            {
                cha->action = cha->body_actions[cha->action_str][cha->action_frame].type;
                cha->action_index = cha->body_actions[cha->action_str][cha->action_frame].frame;
            }
        }
    }
}

void animate_afterimage(AfterImage *aft, Character *cha, entt::entity ent)
{
    auto action = cha->action;
    auto weaponinfo = World::registry->try_get<WeaponInfo>(ent);
    auto afterImage_index = AfterImage::afterImage_index(weaponinfo->reqLevel);
    auto &info = AfterImage::afterimages[weaponinfo->afterImage][afterImage_index][action];
    uint8_t index = info.index;
    auto &aspr = aft->aspr;
    if (cha->action_index == index || aft->animate)
    {
        aft->animate = true;
        if (animate_sprite(&aspr) == false)
        {
            aft->animated = true;
        }
        if (ent == Player::ent && aft->hit == false)
        {
            aft->hit = true;
            if (std::ranges::contains(weaponinfo->attack_stances[weaponinfo->attack], action))
            {
                if (WeaponInfo::if_long_range_weapon(weaponinfo->attack))
                {
                    // 远程
                    load_ball();
                    // play sound
                    Sound::push(AfterImage::sounds[weaponinfo->sfx][0]);
                    return;
                }
            }
            auto atk = World::registry->try_get<Attack>(ent);
            AttackWarp atkw;
            int random = std::rand() % 3;
            switch (random)
            {
            case 0:
            {
                auto hit = aft->hits[weaponinfo->afterImage + u"1"];
                atkw = AttackWarp(aft->info.lt, aft->info.rb, hit);
            }
            break;
            case 1:
            {
                auto hit = aft->hits[weaponinfo->afterImage + u"2"];
                atkw = AttackWarp(aft->info.lt, aft->info.rb, hit);
            }
            break;
            case 2:
            {
                // hit = aft->hits[weaponinfo->afterImage + u"F"];
                auto hit = aft->hits[weaponinfo->afterImage + u"2"];
                atkw = AttackWarp(aft->info.lt, aft->info.rb, hit);
            }
            break;
            }
            atkw.p = &World::registry->try_get<Transform>(ent)->position;
            atk->atks.push_back(atkw);
            // play sound
            Sound::push(AfterImage::sounds[weaponinfo->sfx][0]);
        }
    }
}

void animate_effect(Effect *eff)
{
    for (auto it = eff->effects.begin(); it != eff->effects.end();)
    {
        auto info = &(*it);
        if (info->delay > Window::dt_now)
        {
            it++;
            continue;
        }
        auto aspr = &info->aspr;
        if (animate_sprite(aspr) == false)
        {
            auto tr = info->tr;
            if (tr != nullptr)
            {
                delete tr;
            }
            it = eff->effects.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return;
}

void animate_portal(Portal *por)
{
    // 更新三段式传送门,这里简单的更新三段式传送门的所有状态
    if (por->index >= 0)
    {
        animate_sprite(por->a[por->index]);
    }
}

void animate_npc(Npc *npc)
{
    if (!animate_sprite(npc->a[npc->index]))
    {
        // 切换npc状态
        auto it = npc->a.find(npc->index);
        auto next = std::next(it);
        if (next != npc->a.end())
        {
            npc->index = next->first;
        }
        else
        {
            npc->index = npc->a.begin()->first;
        }
    }
}

void animate_mob(Mob *mob, entt::entity ent)
{
    if (mob->state != Mob::State::REMOVE)
    {
        if (!animate_sprite(&mob->a[mob->index]))
        {
            if (mob->state == Mob::State::HIT)
            {
                if (mob->a.contains(u"stand"))
                {
                    mob->state = Mob::State::STAND;
                    mob->index = u"stand";
                }
                else if (mob->a.contains(u"fly"))
                {
                    mob->state = Mob::State::FLY;
                    mob->index = u"fly";
                }
            }
            else if (mob->state == Mob::State::DIE)
            {
                mob->state = Mob::State::REMOVE;
                if (mob->a.contains(u"stand"))
                {
                    mob->index = u"stand";
                }
                else if (mob->a.contains(u"fly"))
                {
                    mob->index = u"fly";
                }
            }
            else if (mob->state == Mob::State::ATTACK)
            {
                // 怪物攻击
                if (World::registry->valid(mob->hit))
                {
                    auto h = World::registry->try_get<Hit>(mob->hit);
                    auto c = World::registry->try_get<Character>(mob->hit);
                    if (h->damage <= 0 && c->invincible_cooldown <= 0)
                    {
                        auto tr = World::registry->try_get<Transform>(ent);
                        mob->atkw.p = &tr->position;
                        hit_effect(&mob->atkw, mob->hit, 1, nullptr);
                    }
                }
                if (mob->a.contains(u"stand"))
                {
                    mob->state = Mob::State::STAND;
                    mob->index = u"stand";
                }
                else if (mob->a.contains(u"fly"))
                {
                    mob->state = Mob::State::FLY;
                    mob->index = u"fly";
                }
            }
            mob->a[mob->index].anim_index = 0;
            mob->a[mob->index].anim_time = 0;
        }
    }
}

void animate_damage(Damage *dam)
{
    for (auto it = dam->damage.begin(); it != dam->damage.end();)
    {
        auto &info = it;
        auto delta_time = Window::delta_time;
        info->alpha -= (float)Window::delta_time * 0.3;
        if (info->alpha <= 0)
        {
            it = dam->damage.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void animate_tomb(Tomb *tomb, Transform *tr)
{
    if (tomb->f.position.y < tomb->l.position.y)
    {
        tomb->f.position.y += 10;
    }
    if (tomb->aspr.anim_size != 1 && !animate_sprite(&tomb->aspr) && tomb->f.position.y >= tomb->l.position.y)
    {
        tomb->aspr = AnimatedSprite(tomb->land);
    }

    const float velocity = 0.05f; // 角速度
    const float radius = 10.0f;   // 半径
    tr->position.x = tomb->l.position.x + radius * std::cos(tr->rotation);
    tr->position.y = tomb->l.position.y - 10 + radius * std::sin(tr->rotation);
    tr->rotation += velocity;
    if (tr->rotation >= 2 * std::numbers::pi)
    {
        tr->rotation -= 2 * std::numbers::pi; // 保持角度在 [0, 2π) 范围内
    }
}

void animate_drop(Drop *dro, Transform *tr)
{
    if (dro->land == false && dro->picker == entt::null && dro->id != u"09000000")
    {
        tr->rotation += (float)Window::delta_time;
    }
    else
    {
        tr->rotation = 0;
    }
    animate_sprite(&dro->aspr);
}

void animate_face(Character *cha)
{
    if (cha->face_type != u"default" && cha->face_max_index >= 1)
    {
        cha->face_time -= Window::delta_time;
        if (cha->face_time <= 0)
        {
            auto index = std::stoi(std::string{cha->face_index.begin(), cha->face_index.end()});
            // 切换表情到下一帧
            if (index < cha->face_max_index)
            {
                index += 1;
                auto index_str = std::to_string(index);
                cha->face_index = std::u16string{index_str.begin(), index_str.end()};
                cha->add_face(cha->face_str, cha->face_type, cha->face_index);
            }
            else
            {
                cha->face_index = u"";
                cha->face_type = u"default";
                player_face_cooldown = 0;
                cha->add_face(cha->face_str, cha->face_type, cha->face_index);
            }
        }
    }
}
