#include "Move.h"
#include "Core/World.h"
#include "Commons/Commons.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

float move_border_limit_x(Move *mv, float x)
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

float move_border_limit_y(Move *mv, float y)
{
    auto ry0 = mv->ry0;
    auto ry1 = mv->ry1;
    if (!(ry0.has_value() && ry1.has_value()))
    {
        ry0 = World::registry->ctx().get<Border>().t;
        ry1 = World::registry->ctx().get<Border>().b;
    }
    if (y < ry0.value() || y > ry1.value())
    {
        y = std::clamp(y, ry0.value(), ry1.value());
    }
    return y;
}

bool move_fall(Move *mv, Transform *tr, float delta_time, int z_index, bool fall_collide, bool wall_collide)
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

    if (auto new_x = move_border_limit_x(mv, new_pos.x); new_pos.x != new_x)
    {
        new_pos.x = new_x;
        mv->hspeed = 0;
    }
    if (auto new_y = move_border_limit_y(mv, new_pos.y); new_pos.y != new_y)
    {
        new_pos.y = new_y;
        mv->vspeed = 0;
    }

    const auto collide_wall = [](FootHold *fh, float hspeed) -> bool
    {
        if (fh->k.has_value() == false)
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
        }
        else
        {
            return true;
        }
    };

    if (wall_collide)
    {
        // 首先和墙面碰撞
        for (auto &e : World::registry->view<WallFootHold>())
        {
            auto fh = World::registry->try_get<FootHold>(e);
            auto collide = intersect(tr->position, new_pos, {(float)fh->x1, (float)fh->y1}, {(float)fh->x2, (float)fh->y2});
            if (collide.has_value() && collide_wall(fh, mv->hspeed))
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

    // 下落
    if (mv->vspeed >= 0)
    {
        if (fall_collide)
        {
            float top_x;
            float top_y = std::numeric_limits<float>::max();
            FootHold *top_fh = nullptr;
            for (auto &e : World::registry->view<FloorFootHold>())
            {
                auto fh = World::registry->try_get<FootHold>(e);
                if (fh->x2 < fh->x1)
                {
                    continue;
                }
                auto collide = intersect(tr->position, new_pos, {(float)fh->x1, (float)fh->y1}, {(float)fh->x2, (float)fh->y2});
                if (collide.has_value())
                {
                    if (collide.value().y < top_y)
                    {
                        top_x = collide.value().x;
                        top_y = collide.value().y;
                        top_fh = fh;
                    }
                }
            }
            if (top_fh != nullptr)
            {
                // 落地
                new_pos.x = std::clamp(top_x, (float)top_fh->l, (float)top_fh->r);
                new_pos.y = top_fh->get_y(new_pos.x).value();
                mv->foo = top_fh;
                mv->hspeed /= 2;
                tr->position = new_pos;
                // switch z
                if (mv->page != top_fh->page || mv->lr != nullptr)
                {
                    mv->lr = nullptr;
                    tr->z = top_fh->page * LAYER_Z + z_index;
                    mv->page = top_fh->page;
                    World::zindex = true;
                }
                return false;
            }
        }
    }
    else if (mv->vspeed < 0)
    {
        for (auto &e : World::registry->view<FloorFootHold>())
        {
            auto fh = World::registry->try_get<FootHold>(e);
            if (fh->x2 < fh->x1 &&
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
    tr->position = new_pos;
    return true;
}

bool move_move(Move *mv, Transform *tr, float friction, float delta_time, bool fall)
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
    if (auto new_x = move_border_limit_x(mv, x); x != new_x)
    {
        x = new_x;
        mv->hspeed = 0;
    }
    auto y = tr->position.y;

    // 人物在fh移动的函数
    const auto walk_fh = [&x, &y, &tr, &mv, &fall](FootHold *next_fh) -> bool
    {
        FootHold *fh = nullptr; // 走到下一个fh
        if (next_fh)
        {
            fh = next_fh;
        }
        else if (fall)
        {
            mv->vspeed = 0;
            tr->position.y = y;
            tr->position.x = x;
            mv->foo = nullptr;
            return false;
        }
        else
        {
            // 禁止掉落
            tr->position.x = mv->hspeed < 0 ? mv->foo->l + 0.1 : mv->foo->r - 0.1;
            tr->position.y = y;
            mv->hspeed = 0;
            return false;
        }
        if (!fh->k.has_value())
        {
            if (y == std::clamp(y, (float)fh->b - 1, (float)fh->b + 1) || y >= (float)fh->b + 1)
            {
                // 撞墙
                tr->position.x = mv->hspeed < 0 ? fh->x1 + 0.1 : fh->x1 - 0.1;
                tr->position.x = std::clamp(tr->position.x, (float)mv->foo->l, (float)mv->foo->r);
                tr->position.y = mv->foo->get_y(tr->position.x).value();
                mv->hspeed = 0;
                return false;
            }
            else if (fall)
            {
                // 楼梯上掉落
                mv->vspeed = 0;
                tr->position.x = mv->hspeed < 0 ? fh->x1 - 0.1 : fh->x1 + 0.1;
                tr->position.y = y;
                mv->foo = nullptr;
                return false;
            }
            else
            {
                // 禁止掉落
                tr->position.x = mv->hspeed < 0 ? fh->x1 + 0.1 : fh->x1 - 0.1;
                tr->position.y = y;
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
