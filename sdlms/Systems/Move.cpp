module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>

module systems;

import components;
import commons;
import core;

void move_run()
{
    auto view = World::registry.view<Move>();
    for (auto &ent : view)
    {
        auto mv = &view.get<Move>(ent);
        if (World::registry.any_of<Player>(ent))
        {
            auto tr = World::registry.try_get<Transform>(ent);
            auto cha = World::registry.try_get<Character>(ent);
            move_player(mv, tr, cha, mv->foo);
        }
    }
}

void move_player(Move *mv, Transform *tr, Character *cha, FootHold *foo)
{
    move_play_flip(tr);
    switch (cha->status)
    {
    case Character::Status::NONE:
    {
        // normal status,can walk or jump
        if (!move_play_walk(mv, tr, foo))
        {
            cha->status = Character::Status::JUMP;
        }
    }
    break;
    case Character::Status::JUMP:
    {
        if (!move_play_fall(mv, tr, foo))
        {
            cha->status = Character::Status::NONE;
        }
    }
    break;
    default:
        break;
    }
}

void move_play_flip(Transform *tr)
{
    if (Input::is_key_held(SDLK_RIGHT))
    {
        tr->flip = 1;
    }
    else if (Input::is_key_held(SDLK_LEFT))
    {
        tr->flip = 0;
    }
}

bool move_play_walk(Move *mv, Transform *tr, FootHold *foo)
{
    if (Input::is_key_held(SDLK_RIGHT))
    {
        mv->hforce = 1400;
    }
    else if (Input::is_key_held(SDLK_LEFT))
    {
        mv->hforce = -1400;
    }
    else
    {
        mv->hforce = 0;
        // 如果没有左右的输入并且速度为0,则可以直接return提高性能
        if (mv->hspeed == 0)
        {
            return true;
        }
    }
    auto delta_time = (float)Window::delta_time / 1000;

    constexpr auto friction = 800; // 摩擦力

    if (mv->hspeed > 0)
    {
        mv->hforce -= friction;
        mv->hforce = std::max(-mv->hspeed / delta_time, mv->hforce);
    }
    else if (mv->hspeed < 0)
    {
        mv->hforce += friction;
        mv->hforce = std::min(-mv->hspeed / delta_time, mv->hforce);
    }

    mv->hspeed += delta_time * mv->hforce;

    if (mv->hspeed_min.has_value())
    {
        mv->hspeed = std::fmax(mv->hspeed, mv->hspeed_min.value());
    }
    if (mv->hspeed_max.has_value())
    {
        mv->hspeed = std::fmin(mv->hspeed, mv->hspeed_max.value());
    }

    auto d_x = mv->hspeed * delta_time;
    auto x = d_x + tr->position.x;
    auto y = tr->position.y;

    auto &fhs = FootHold::fhs;

    // 人物在fh移动的函数
    auto walk_fh = [&fhs, &foo, &x, &y, &tr, &mv](int next_fh) -> bool
    {
        FootHold *fh = nullptr; // 走到下一个fh
        if (fhs.contains(next_fh))
        {
            fh = fhs.find(next_fh)->second;
        }
        else
        {
            mv->vspeed = 0;
            tr->position.y = y;
            tr->position.x = x;
            return false;
        }
        if (!fh->k.has_value())
        {
            if (y == std::clamp(y, (float)fh->b - 1, (float)fh->b + 1))
            {
                // 撞墙
                x = tr->position.x;
                tr->position.y = foo->get_y(tr->position.x).value();
                mv->hspeed = 0;
                return false;
            }
            else
            {
                // 楼梯上掉落
                mv->vspeed = 0;
                tr->position.y = y;
                tr->position.x = x;
                return false;
            }
        }
        foo = fh;
        return true;
    };

    // 往左走
    while (x < foo->l)
    {
        int next_fh = std::abs(foo->prev);
        if (walk_fh(next_fh) == false)
        {
            // 从fh掉落,撞墙
            return false;
        }
    }
    // 往右走
    while (x > foo->r)
    {
        int next_fh = std::abs(foo->next);
        if (walk_fh(next_fh) == false)
        {
            // 从fh掉落,撞墙
            return false;
        }
    }
    // 地面上
    tr->position.x = x;
    tr->position.y = foo->get_y(x).value();
    return true;
}

bool move_play_fall(Move *mv, Transform *tr, FootHold *foo)
{
    if (Input::is_key_held(SDLK_RIGHT))
    {
        mv->hspeed += 0.3f;
    }
    else if (Input::is_key_held(SDLK_LEFT))
    {
        mv->hspeed -= 0.3f;
    }

    auto delta_time = (float)Window::delta_time / 1000;

    // 默认重力为2000
    mv->vspeed += delta_time * 2000;
    if (mv->vspeed_min.has_value())
    {
        mv->vspeed = std::fmax(mv->vspeed, mv->vspeed_min.value());
    }
    if (mv->vspeed_max.has_value())
    {
        mv->vspeed = std::fmin(mv->vspeed, mv->vspeed_max.value());
    }

    auto d_x = mv->hspeed * delta_time;
    auto d_y = mv->vspeed * delta_time;

    auto new_pos = tr->position + SDL_FPoint{(float)d_x, (float)d_y};

    auto &fhs = FootHold::fhs;

    auto collide_wall = [&fhs](FootHold *fh, float hspeed) -> bool
    {
        if (hspeed > 0 && fh->y1 > fh->y2)
        {
            while (fhs.contains(fh->prev))
            {
                fh = fhs.find(fh->prev)->second;
                if (fh->k.has_value())
                {
                    return true;
                }
            }
        }
        else if (hspeed < 0 && fh->y1 < fh->y2)
        {
            while (fhs.contains(fh->next))
            {
                fh = fhs.find(fh->next)->second;
                if (fh->k.has_value())
                {
                    return true;
                }
            }
        }
        return false;
    };

    // 下落
    if (mv->vspeed >= 0)
    {
        auto view = World::registry.view<FootHold>();
        for (auto &ent : view)
        {
            auto fh = &view.get<FootHold>(ent);
            auto collide = intersect(tr->position, new_pos, {(float)fh->x1, (float)fh->y1}, {(float)fh->x2, (float)fh->y2});
            if (collide.has_value())
            {
                if (!fh->k.has_value())
                {
                    // 判断墙面碰撞方向
                    if (collide_wall(fh, mv->hspeed))
                    {
                        new_pos.x = tr->position.x;
                        mv->hspeed = 0;
                    }
                }
                else
                {
                    // 落地
                    new_pos.x = std::clamp((float)collide.value().x, (float)fh->l, (float)fh->r);
                    new_pos.y = fh->get_y(new_pos.x).value();
                    mv->foo = fh;
                    tr->position = new_pos;
                    return false;
                }
            }
        }
    }
    else if (mv->vspeed < 0)
    {
        auto view = World::registry.view<FootHold>();
        for (auto &ent : view)
        {
            auto fh = &view.get<FootHold>(ent);
            if (!fh->k.has_value())
            {
                // 墙
                auto collide = intersect(tr->position, new_pos, {(float)fh->x1, (float)fh->y1}, {(float)fh->x2, (float)fh->y2});
                if (collide.has_value())
                {
                    if (collide_wall(fh, mv->hspeed))
                    {
                        new_pos.x = tr->position.x;
                        mv->hspeed = 0;
                    }
                }
            }
            else
            {
                if (fh->k == 0 &&
                    fh->x2 < fh->x1 &&
                    (fh->zmass == 0 ||
                     (foo != nullptr &&
                      fh->zmass == foo->zmass)))
                {
                    // top floor
                    auto collide = intersect(tr->position, new_pos, {(float)fh->x1, (float)fh->y1}, {(float)fh->x2, (float)fh->y2});
                    if (collide.has_value())
                    {
                        if (collide_wall(fh, mv->hspeed))
                        {
                            new_pos.y = tr->position.y;
                            mv->hspeed = 0;
                            mv->vspeed = 0;
                        }
                    }
                }
            }
        }
    }
    tr->position = new_pos;
    return true;
}

inline std::optional<SDL_FPoint> intersect(SDL_FPoint p1, SDL_FPoint p2, SDL_FPoint p3, SDL_FPoint p4)
{

    // 快速排斥实验
    if ((p1.x > p2.x ? p1.x : p2.x) < (p3.x < p4.x ? p3.x : p4.x) ||
        (p1.y > p2.y ? p1.y : p2.y) < (p3.y < p4.y ? p3.y : p4.y) ||
        (p3.x > p4.x ? p3.x : p4.x) < (p1.x < p2.x ? p1.x : p2.x) ||
        (p3.y > p4.y ? p3.y : p4.y) < (p1.y < p2.y ? p1.y : p2.y))
    {
        return std::nullopt;
    }
    // 跨立实验
    if ((((p1.x - p3.x) * (p4.y - p3.y) - (p1.y - p3.y) * (p4.x - p3.x)) *
         ((p2.x - p3.x) * (p4.y - p3.y) - (p2.y - p3.y) * (p4.x - p3.x))) > 0 ||
        (((p3.x - p1.x) * (p2.y - p1.y) - (p3.y - p1.y) * (p2.x - p1.x)) *
         ((p4.x - p1.x) * (p2.y - p1.y) - (p4.y - p1.y) * (p2.x - p1.x))) > 0)
    {
        return std::nullopt;
    }

    auto x = ((p1.y - p3.y) * (p2.x - p1.x) * (p4.x - p3.x) + p3.x * (p4.y - p3.y) * (p2.x - p1.x) - p1.x * (p2.y - p1.y) * (p4.x - p3.x)) / ((p4.x - p3.x) * (p1.y - p2.y) - (p2.x - p1.x) * (p3.y - p4.y));
    auto y = (p2.y * (p1.x - p2.x) * (p4.y - p3.y) + (p4.x - p2.x) * (p4.y - p3.y) * (p1.y - p2.y) - p4.y * (p3.x - p4.x) * (p2.y - p1.y)) / ((p1.x - p2.x) * (p4.y - p3.y) - (p2.y - p1.y) * (p3.x - p4.x));

    return SDL_FPoint{x, y};
}
