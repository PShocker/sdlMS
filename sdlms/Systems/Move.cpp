module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

module systems;

import components;
import commons;
import core;
import entities;

float move_border_limit(Move *mv, float x)
{
    auto rx0 = mv->rx0;
    auto rx1 = mv->rx1;
    if (!(rx0.has_value() && rx1.has_value()))
    {
        rx0 = World::registry->ctx().get<Border>().l;
        rx1 = World::registry->ctx().get<Border>().r;
    }
    if (x < rx0.value() || x > rx1.value())
    {
        x = std::clamp(x, rx0.value(), rx1.value());
    }
    return x;
}

bool move_fall(Move *mv, Transform *tr, float delta_time, bool fall_collide)
{
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

    if (auto new_x = move_border_limit(mv, new_pos.x); new_pos.x != new_x)
    {
        new_pos.x = new_x;
        mv->hspeed = 0;
    }

    auto collide_wall = [](FootHold *fh, float hspeed) -> bool
    {
        if (hspeed > 0 && fh->y1 > fh->y2)
        {
            while (fh->prev)
            {
                fh = fh->prev;
                if (fh->k.has_value())
                {
                    return true;
                }
            }
        }
        else if (hspeed < 0 && fh->y1 < fh->y2)
        {
            while (fh->next)
            {
                fh = fh->next;
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
        auto view = World::registry->view<FootHold>();
        for (auto &e : view)
        {
            auto fh = &view.get<FootHold>(e);
            auto collide = intersect(tr->position, new_pos, {(float)fh->x1, (float)fh->y1}, {(float)fh->x2, (float)fh->y2});
            if (collide.has_value())
            {
                if (!fh->k.has_value())
                {
                    // 判断墙面碰撞方向
                    if (collide_wall(fh, mv->hspeed))
                    {
                        if (mv->hspeed < 0)
                        {
                            new_pos.x = fh->x1 + 0.1;
                        }
                        else
                        {
                            new_pos.x = fh->x1 - 0.1;
                        }
                        mv->hspeed = 0;
                    }
                }
                else if (fall_collide)
                {
                    // 落地
                    new_pos.x = std::clamp((float)collide.value().x, (float)fh->l, (float)fh->r);
                    new_pos.y = fh->get_y(new_pos.x).value();
                    mv->foo = fh;
                    mv->hspeed /= 2;
                    tr->position = new_pos;
                    // switch z
                    if (mv->page != fh->page)
                    {
                        tr->z = fh->page * LAYER_Z + CHARACTER_Z;
                        mv->page = fh->page;
                        World::zindex = true;
                    }

                    return false;
                }
            }
        }
    }
    else if (mv->vspeed < 0)
    {
        auto view = World::registry->view<FootHold>();
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
                        if (mv->hspeed < 0)
                        {
                            new_pos.x = fh->x1 + 0.1;
                        }
                        else
                        {
                            new_pos.x = fh->x1 - 0.1;
                        }
                        mv->hspeed = 0;
                    }
                }
            }
            else
            {
                if (fh->k == 0 &&
                    fh->x2 < fh->x1 &&
                    (fh->zmass == 0 ||
                     fh->zmass == mv->zmass))
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

bool move_move(Move *mv, Transform *tr, int friction, float delta_time)
{
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
    if (auto new_x = move_border_limit(mv, x); x != new_x)
    {
        x = new_x;
        mv->hspeed = 0;
    }
    auto y = tr->position.y;

    // 人物在fh移动的函数
    auto walk_fh = [&x, &y, &tr, &mv](FootHold *next_fh) -> bool
    {
        FootHold *fh = nullptr; // 走到下一个fh
        if (next_fh)
        {
            fh = next_fh;
        }
        else
        {
            mv->vspeed = 0;
            tr->position.y = y;
            tr->position.x = x;
            mv->foo = nullptr;
            return false;
        }
        if (!fh->k.has_value())
        {
            if (y == std::clamp(y, (float)fh->b - 1, (float)fh->b + 1))
            {
                // 撞墙
                if (mv->hspeed < 0)
                {
                    tr->position.x = fh->x1 + 0.1;
                }
                else
                {
                    tr->position.x = fh->x1 - 0.1;
                }
                tr->position.y = mv->foo->get_y(tr->position.x).value();
                mv->hspeed = 0;
                return false;
            }
            else
            {
                // 楼梯上掉落
                mv->vspeed = 0;
                tr->position.y = y;
                tr->position.x = x;
                mv->foo = nullptr;
                return false;
            }
        }
        mv->foo = fh;
        return true;
    };

    // 往左走
    while (x < mv->foo->l)
    {
        auto prev_fh = mv->foo->prev;
        if (walk_fh(prev_fh) == false)
        {
            return false;
        }
    }
    // 往右走
    while (x > mv->foo->r)
    {
        auto next_fh = mv->foo->next;
        if (walk_fh(next_fh) == false)
        {
            return false;
        }
    }
    // 地面上
    tr->position.x = x;
    tr->position.y = mv->foo->get_y(x).value();
    return true;
}
