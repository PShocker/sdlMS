#include "MobStateMachine.h"
#include "Move.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "Entities/Entities.h"

void mob_statemachine_run()
{
    auto view = World::registry->view<Mob>();
    for (auto ent : view)
    {
        auto mob = World::registry->try_get<Mob>(ent);
        if (mob->state == Mob::State::REMOVE)
        {
            mob_revive(ent, Window::delta_time);
            continue;
        }
        if (mob->state != Mob::State::REMOVE && mob->state != Mob::State::DIE &&
            mob_hit(World::registry->try_get<Hit>(ent), ent))
        {
            continue;
        }
        mob_statemachine(ent, (float)Window::delta_time / 1000);
    }
}

void mob_statemachine(entt::entity ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto mob = World::registry->try_get<Mob>(ent);

    auto state = mob->state;
    switch (state)
    {
    case Mob::State::STAND:
    {
        if (mob_attack(mob, tr))
        {
            mob->state = Mob::State::ATTACK;
        }
    }
    break;
    case Mob::State::MOVE:
    {
        mob_flip(mv, tr);
        if (mob_attack(mob, tr))
        {
            mob->state = Mob::State::ATTACK;
            break;
        }
        if (mob_jump(mob, mv))
        {
            mob->state = Mob::State::JUMP;
        }
        else
        {
            mob->state = mob_move(mob, mv, tr, state, delta_time);
        }
    }
    break;
    case Mob::State::FLY:
    {
        mob_flip(mv, tr);
        mob->state = mob_fly(mob, mv, tr, state, delta_time);
    }
    break;
    case Mob::State::JUMP:
    case Mob::State::HIT:
    {
        mob_fall(mob, mv, tr, delta_time);
    }
    break;
    case Mob::State::DIE:
    {
    }
    break;
    }
    mob->state = mob_active(mob, mv, tr, mob->state, delta_time);
    mob_action(mob, mv, tr, state, mob->state);
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

int mob_move(Mob *mob, Move *mv, Transform *tr, int state, float delta_time)
{
    move_move(mv, tr, 0, delta_time, false);
    return state;
}

int mob_fly(Mob *mob, Move *mv, Transform *tr, int state, float delta_time)
{
    move_fall(mv, tr, delta_time, tr->z, false, false);
    return state;
}

void mob_action(Mob *mob, Move *mv, Transform *tr, int state, int new_state)
{
    if (state != new_state)
    {
        int action = new_state;
        switch (action)
        {
        case Mob::State::STAND:
        {
            mob->tick = std::rand() % 100 + 2110;
            mob->index = u"stand";
            mv->hspeed = 0;
        }
        break;
        case Mob::State::MOVE:
        {
            mob->tick = std::rand() % 100 + 2390;
            mob->index = u"move";
            mob_set_hspeed(mob, mv, tr);
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
        case Mob::State::ATTACK:
        {
            mob->index = u"attack1";
        }
        break;
        }
        mob->a[mob->index].anim_index = 0;
        mob->a[mob->index].anim_time = 0;
    }
}

bool mob_hit(Hit *hit, entt::entity ent)
{
    if (hit->damage > 0)
    {
        auto mv = World::registry->try_get<Move>(ent);
        auto tr = World::registry->try_get<Transform>(ent);
        auto mob = World::registry->try_get<Mob>(ent);

        auto count = hit->count;
        mob->hp -= hit->damage;
        mob->hit = hit->owner;

        hit->damage = 0;
        hit->count = 0;

        for (int i = 0; i < count; i++)
        {
            // 怪物被攻击音效
            if (hit->souw)
            {
                Sound::push(hit->souw, i * 140);
            }
            else if (mob->sounds.contains(u"Damage"))
            {
                Sound::push(mob->sounds[u"Damage"], i * 140);
            }
        }

        if (mob->hp > 0)
        {
            // 获取玩家位置,并让怪物转身和后退
            if (mob->state == Mob::State::FLY)
            {
                mob_hit_move(hit, ent);
                mob_fly(mob, mv, tr, mob->state, 0.15);
            }
            else if (mv->foo)
            {
                mob_hit_move(hit, ent);
                mob_move(mob, mv, tr, mob->state, 0.15);
            }
            mob->tick = 0;
            mob->state = Mob::State::HIT;
            mob->index = u"hit1";
        }
        else
        {
            mob->state = Mob::State::DIE;
            mob->index = u"die1";
            // 怪物死亡音效
            if (mob->sounds.contains(u"Die"))
            {
                Sound::push(mob->sounds[u"Die"], 200);
            }
            // 爆金币
            mob_drop(mob, tr);

            mob->revive = mob->revive_time + Window::dt_now;
            return true;
        }
    }
    return false;
}

void mob_hit_move(Hit *hit, entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto mob = World::registry->try_get<Mob>(ent);
    auto hit_x = hit->x;
    auto mob_x = tr->position.x;
    if (mob_x < hit_x)
    {
        tr->flip = 1;
        mv->hspeed = mv->hspeed_min.value();
    }
    else if (mob_x > hit_x)
    {
        tr->flip = 0;
        mv->hspeed = mv->hspeed_max.value();
    }
    return;
}

void mob_drop(Mob *mob, Transform *tr)
{
    std::vector<DropInfo> drops = {{u"09000000", (unsigned int)std::rand() % 200}};
    for (auto it : mob->drops[mob->id])
    {
        auto id = std::to_string(it);
        drops.push_back({std::u16string{id.begin(), id.end()}, 1});
    }
    load_drops(&drops, tr->position.x, tr->position.y, tr->z / LAYER_Z);
    Sound::push(Sound(u"Game.img/DropItem", 300));
    World::zindex = true;
}

bool mob_fall(Mob *mob, Move *mv, Transform *tr, float delta_time)
{
    if (mv->foo)
    {
        return false;
    }
    else
    {
        // 默认重力为2000
        mv->vspeed += delta_time * 2000;
        if (move_fall(mv, tr, delta_time, tr->z % LAYER_Z))
        {
            return true;
        }
        else
        {
            // 落地
            mob->tick = 0;
            return false;
        }
    }
}

int mob_active(Mob *mob, Move *mv, Transform *tr, int state, float delta_time)
{
    if (!(state == Mob::State::ATTACK || state == Mob::State::HIT || state == Mob::State::DIE ||
          state == Mob::State::REMOVE || mv->foo == nullptr))
    {
        mob->tick -= delta_time * 1000;
        if (mob->tick < 0)
        {
            if (state == Mob::State::FLY)
            {
                mob->tick = std::rand() % 100 + 200;
                mob_set_hspeed(mob, mv, tr);
                mob_set_vspeed(mob, mv, tr);
                if (World::registry->valid(mob->hit))
                {
                    mv->rx0 = std::nullopt;
                    mv->rx1 = std::nullopt;
                    mv->ry0 = std::nullopt;
                    mv->ry1 = std::nullopt;
                }
                else
                {
                    if (tr->position.y <= mv->ry0.value())
                    {
                        mv->vspeed = mv->hspeed_max.value();
                    }
                    else if (tr->position.y >= mv->ry1.value())
                    {
                        mv->vspeed = mv->hspeed_min.value();
                    }
                }
            }
            else if (mv->foo)
            {
                if (World::registry->valid(mob->hit))
                {
                    mob->tick = std::rand() % 100 + 200;
                    mob_set_hspeed(mob, mv, tr);
                    state = Mob::State::MOVE;
                }
                else
                {
                    // 只有在地面才可以切换状态
                    //  怪物状态切换 STAND MOVE
                    mob->tick = std::rand() % 100 + 240;
                    int random = std::rand() % 2;
                    switch (random)
                    {
                    case 0:
                        state = Mob::State::STAND;
                        mv->hspeed = 0;
                        break;
                    case 1:
                        state = Mob::State::MOVE;
                        mob_set_hspeed(mob, mv, tr);
                        break;
                    }
                }
            }
        }
    }
    return state;
}

bool mob_revive(entt::entity ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto mob = World::registry->try_get<Mob>(ent);

    if (mob->revive <= Window::dt_now)
    {
        if (mob->a.contains(u"fly"))
        {
            mob->state = Mob::State::FLY;
        }
        else
        {
            mob->state = Mob::State::STAND;
        }
        return true;
    }
    else if (mob->revive <= Window::dt_now + mob->revive_alpha_time + 100)
    {
        auto hit = World::registry->try_get<Hit>(ent);
        hit->damage = 0;
        hit->count = 0;

        mob->hp = 100;
        mob->hit = entt::null;
        if (mob->a.contains(u"fly"))
        {
            mv->ry0 = mob->init_y - 40;
            mv->ry1 = mob->init_y + 40;
        }
        mv->foo = mob->init_fh;
        tr->position.x = mob->init_x;
        tr->position.y = mob->init_y;
    }
    return false;
}

bool mob_attack(Mob *mob, Transform *tr)
{
    if (World::registry->valid(mob->hit) && mob->a.contains(u"attack1"))
    {
        // 怪物攻击
        auto h_tr = World::registry->try_get<Transform>(mob->hit);
        if (std::abs(h_tr->position.x - tr->position.x) < 200 &&
            std::abs(h_tr->position.y - tr->position.y) < 15)
        {
            return true;
        }
    }
    return false;
}

bool mob_jump(Mob *mob, Move *mv)
{
    if (mob->a.contains(u"jump"))
    {
        FootHold *fh = nullptr;
        // 判断移动方向
        if (mv->hspeed > 0)
        {
            // 向右移动
            fh = mv->foo->next;
        }
        else
        {
            // 向左移动
            fh = mv->foo->prev;
        }
        if (fh == nullptr || fh->k.has_value() == false)
        {
            // 有概率起跳
            int random = std::rand() % 35;
            if (random == 0)
            {
                mob->index = u"jump";
                mob->state = Mob::State::JUMP;

                mv->foo = nullptr;
                mv->vspeed = -420;
                return true;
            }
        }
    }
    return false;
}

void mob_set_hspeed(Mob *mob, Move *mv, Transform *tr)
{
    auto ent = mob->hit;
    if (World::registry->valid(ent))
    {
        auto m_tr = tr;
        auto h_tr = World::registry->try_get<Transform>(ent);
        if (h_tr != nullptr)
        {
            mv->hspeed = (h_tr->position.x > m_tr->position.x)
                             ? mv->hspeed_max.value()
                             : mv->hspeed_min.value();
        }
    }
    else
    {
        int random = std::rand() % 2;
        switch (random)
        {
        case 0:
        {
            mv->hspeed = mv->hspeed_min.value();
            break;
        }
        case 1:
        {
            mv->hspeed = mv->hspeed_max.value();
            break;
        }
        }
    }
    return;
}

void mob_set_vspeed(Mob *mob, Move *mv, Transform *tr)
{
    auto ent = mob->hit;
    if (World::registry->valid(ent))
    {
        auto m_tr = tr;
        auto h_tr = World::registry->try_get<Transform>(ent);
        if (h_tr != nullptr)
        {
            mv->vspeed = (h_tr->position.y > m_tr->position.y)
                             ? mv->hspeed_max.value()
                             : mv->hspeed_min.value();
        }
    }
    else
    {
        int random = std::rand() % 2;
        switch (random)
        {
        case 0:
        {
            mv->vspeed = mv->hspeed_min.value();
            break;
        }
        case 1:
        {
            mv->vspeed = mv->hspeed_max.value();
            break;
        }
        }
    }
    return;
}
