module;

#include "entt/entt.hpp"

module systems;

import components;
import commons;
import core;
import entities;

void mob_statemachine_run()
{
    auto view = World::registry->view<Mob>();
    for (auto ent : view)
    {
        mob_statemachine(&ent, (float)Window::delta_time / 1000);
    }
}

void mob_statemachine(entt::entity *ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(*ent);
    auto tr = World::registry->try_get<Transform>(*ent);
    auto mob = World::registry->try_get<Mob>(*ent);

    auto state = mob->state;
    switch (state)
    {
    case Mob::State::STAND:
    {
        mob->state = mob_stand(mob, mv, tr, state, delta_time);
    }
    break;
    case Mob::State::MOVE:
    {
        mob->state = mob_move(mob, mv, tr, state, delta_time);
    }
    break;
    case Mob::State::HIT:
    {
    }
    break;
    }
    mob_border_limit(mv, tr);
    mob_action(mob, mv, state, mob->state);
}

int mob_stand(Mob *mob, Move *mv, Transform *tr, int state, float delta_time)
{
    mob->tick -= delta_time;
    if (mob->tick <= 0)
    {
        // 怪物状态切换 STAND MOVE
        int random = std::rand() % 2;
        switch (random)
        {
        case 0:
            state = Mob::State::STAND;
            break;
        case 1:
            state = Mob::State::MOVE;
            break;
        }
    }
    return state;
}

int mob_move(Mob *mob, Move *mv, Transform *tr, int state, float delta_time)
{
    mob->tick -= delta_time;
    if (mob->tick > 0)
    {
        auto d_x = mv->hspeed * delta_time;
        auto x = d_x + tr->position.x;
        auto y = tr->position.y;
        // 怪物在fh移动的函数
        auto walk_fh = [&x, &y, &tr, &mv, &state](FootHold *next_fh) -> bool
        {
            FootHold *fh = nullptr; // 走到下一个fh
            if (next_fh)
            {
                fh = next_fh;
            }
            else
            {
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
                return state;
            }
        }
        // 往右走
        while (x > mv->foo->r)
        {
            auto next_fh = mv->foo->next;
            if (walk_fh(next_fh) == false)
            {
                return state;
            }
        }
        // 地面上
        tr->position.x = x;
        tr->position.y = mv->foo->get_y(x).value();
        return state;
    }
    else
    {
        // 怪物状态切换 STAND MOVE
        int random = std::rand() % 2;
        switch (random)
        {
        case 0:
            state = Mob::State::STAND;
            break;
        case 1:
            state = Mob::State::MOVE;
            break;
        }
    }
    return state;
}

void mob_action(Mob *mob, Move *mv, int state, int new_state)
{
    if (state != new_state)
    {
        int action = new_state;
        switch (action)
        {
        case Mob::State::STAND:
            mob->index = u"stand";
            break;
        case Mob::State::MOVE:
        {
            mob->tick=200;
            mob->index = u"move";
            int random = std::rand() % 2;
            switch (random)
            {
            case 0:
                mv->hspeed = std::abs(mv->hspeed);
                break;
            case 1:
                mv->hspeed = -std::abs(mv->hspeed);
                break;
            }
        }
        break;
        case Mob::State::HIT:
            mob->index = u"hit1";
            break;
        }
        mob->a[mob->index]->anim_index = 0;
        mob->a[mob->index]->anim_time = 0;
    }
}

void mob_border_limit(Move *mv, Transform *tr)
{
    auto rx0 = mv->rx0;
    auto rx1 = mv->rx1;
    if (!(rx0.has_value() && rx1.has_value()))
    {
        rx0 = World::registry->ctx().get<Border>().l;
        rx1 = World::registry->ctx().get<Border>().r;
    }
    if (tr->position.x < rx0.value() || tr->position.x > rx1.value())
    {
        tr->position.x = std::clamp(tr->position.x, rx0.value(), rx1.value());
        mv->hspeed = 0;
        if (mv->foo)
        {
            tr->position.y = mv->foo->get_y(tr->position.x).value();
        }
    }
}
