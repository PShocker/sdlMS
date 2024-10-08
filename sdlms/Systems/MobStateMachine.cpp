module;

#include "entt/entt.hpp"

module systems;

import components;
import commons;
import core;
import entities;
import :move;

void mob_statemachine_run()
{
    auto view = World::registry->view<Mob>();
    for (auto ent : view)
    {
        auto mob = World::registry->try_get<Mob>(ent);
        if (mob->state == Mob::State::REMOVE)
        {
            continue;
        }
        if (auto hit = World::registry->try_get<Hit>(ent))
        {
            mob_hit(hit, &ent);
            World::registry->remove<Hit>(ent);
            continue;
        }
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
        mob_flip(mv, tr);
        mob->state = mob_move(mob, mv, tr, state, delta_time);
    }
    break;
    case Mob::State::JUMP:
    {
        if (!mob_fall(mv, tr, delta_time))
        {
            mob->state = Mob::State::MOVE;
        }
    }
    break;
    case Mob::State::HIT:
    {
    }
    break;
    case Mob::State::DIE:
    {
    }
    break;
    }
    mob_action(mob, mv, state, mob->state);
}

void mob_flip(Move *mv, Transform *tr)
{
    if (mv->hspeed > 0)
    {
        tr->flip = 1;
    }
    else if (mv->hspeed < 0)
    {
        tr->flip = 0;
    }
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
        if (!move_move(mv, tr, 0, delta_time))
        {
            state = Mob::State::JUMP;
        }
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
        {
            mob->tick = std::rand() % 100 + 10;
            mob->index = u"stand";
            mv->hspeed = 0;
        }
        break;
        case Mob::State::MOVE:
        {
            mob->tick = std::rand() % 100 + 90;
            mob->index = u"move";
            int random = std::rand() % 2;
            switch (random)
            {
            case 0:
                mv->hspeed = mv->hspeed_min.value();
                break;
            case 1:
                mv->hspeed = mv->hspeed_max.value();
                break;
            }
        }
        break;
        case Mob::State::JUMP:
        {
            if (mob->a.contains(u"jump"))
            {
                mob->index = u"jump";
            }
        }
        break;
        case Mob::State::HIT:
        {
            mob->index = u"hit1";
        }
        break;
        }
        mob->a[mob->index]->anim_index = 0;
        mob->a[mob->index]->anim_time = 0;
    }
}

bool mob_hit(Hit *hit, entt::entity *ent)
{
    auto mv = World::registry->try_get<Move>(*ent);
    auto tr = World::registry->try_get<Transform>(*ent);
    auto mob = World::registry->try_get<Mob>(*ent);

    mob->hp -= hit->damage;
    if (mob->hp > 0)
    {
        mob->state = Mob::State::HIT;
        mob->index = u"hit1";

        // 获取玩家位置,并让怪物转身和后退
        auto hit_x = hit->x;
        auto mob_x = tr->position.x;
        if (mob_x < hit_x)
        {
            tr->flip = 1;
            mv->hspeed = mv->hspeed_min.value();
            mob_move(mob, mv, tr, mob->state, 0.15);
        }
        else if (mob_x > hit_x)
        {
            tr->flip = 0;
            mv->hspeed = mv->hspeed_max.value();
            mob_move(mob, mv, tr, mob->state, 0.15);
        }

        // 怪物被攻击音效
        if (mob->sounds.contains(u"Damage"))
        {
            auto souw = mob->sounds[u"Damage"];
            Sound sou;
            sou.souw = souw;
            Sound::sound_list.push_back(sou);
        }
    }
    else
    {
        mob->state = Mob::State::DIE;
        mob->index = u"die1";

        // 怪物死亡音效
        if (mob->sounds.contains(u"Die"))
        {
            auto souw = mob->sounds[u"Die"];
            Sound sou;
            sou.souw = souw;
            Sound::sound_list.push_back(sou);
        }

        // 爆金币
        mob_drop(mob, tr);
        return false;
    }
    return true;
}

void mob_drop(Mob *mob, Transform *tr)
{
    load_drop(u"09000000", std::rand() % 200, tr->position.x, tr->position.y, tr->z);
    World::zindex = true;

    Sound::sound_list.push_back(Sound(u"Game.img/DropItem"));
}

bool mob_fall(Move *mv, Transform *tr, float delta_time)
{
    // 默认重力为2000
    mv->vspeed += delta_time * 2000;

    return move_fall(mv, tr, delta_time);
}
