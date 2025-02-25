#include "SummonStateMachine.h"
#include "PlayerSummon/PlayerSummon.h"
#include "Move.h"

void summon_statemachine_run()
{
    for (auto ent : World::registry->view<Summon>())
    {
        auto sum = World::registry->try_get<Summon>(ent);
        if (sum->destory > Window::dt_now)
        {
            if (PlayerSummon::Summons.contains(sum->id))
            {
                PlayerSummon::Summons.at(sum->id)(ent);
            }
            else if (!World::registry->valid(sum->owner))
            {
                sum->destory = Window::dt_now;
            }
            else
            {
                summon_statemachine(ent, (float)Window::delta_time / 1000);
            }
        }
        else if (sum->a.contains(u"die"))
        {
            sum->state = summon_action(sum, sum->state, Summon::State::DIE);
        }
        else
        {
            World::destory.push_back(ent);
            World::zindex = true;
        }
    }
}

void summon_statemachine(entt::entity ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto sum = World::registry->try_get<Summon>(ent);

    auto state = sum->state;
    switch (state)
    {
    case Summon::State::INIT:
    {
    }
    break;
    case Summon::State::STAND:
    case Summon::State::MOVE:
    {
        if (auto e = summon_attack(tr); World::registry->valid(e))
        {
            // 转向
            auto m_tr = World::registry->try_get<Transform>(e);
            auto s_tr = tr;
            s_tr->flip = m_tr->position.x > s_tr->position.x ? 1 : 0;
            sum->state = Summon::State::ATTACK;
            break;
        }
        else
        {
            summon_flip(mv, tr);
            sum->state = summon_move(sum, mv, tr, state, delta_time);
        }
    }
    break;
    case Summon::State::FLY:
    {
    }
    break;
    case Summon::State::ATTACK:
    {
    }
    break;
    case Summon::State::DIE:
    {
    }
    break;
    }
    summon_action(sum, state, sum->state);
}

void summon_flip(Move *mv, Transform *tr)
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

entt::entity summon_attack(Transform *tr)
{
    for (auto ent : World::registry->view<Damage, Mob>())
    {
        auto mob = World::registry->try_get<Mob>(ent);
        if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
        {
            auto m_tr = World::registry->try_get<Transform>(ent);
            if (std::abs(m_tr->position.x - tr->position.x) <= 200 &&
                std::abs(m_tr->position.y - tr->position.y) <= 35)
            {
                return ent;
            }
        }
    }
    return entt::null;
}

int summon_move(Summon *sum, Move *mv, Transform *tr, int state, float delta_time)
{
    // 跟随玩家
    if (World::registry->valid(sum->owner))
    {
        auto o_tr = World::registry->try_get<Transform>(sum->owner);
        auto s_tr = tr;
        // 先判断玩家周围是否存在怪物
        if (auto e = summon_attack(o_tr); World::registry->valid(e))
        {
            auto m_tr = World::registry->try_get<Transform>(e);
            if (m_tr->position.x > s_tr->position.x + 200)
            {
                mv->hspeed = mv->hspeed_max.value();
            }
            else if (m_tr->position.x < s_tr->position.x - 200)
            {
                mv->hspeed = mv->hspeed_min.value();
            }
        }
        else
        {
            state = summon_follow(sum, mv, tr);
        }
        auto foo = mv->foo;
        if (!move_move(mv, tr, 0, delta_time) && mv->foo == nullptr)
        {
            // 不掉落
            mv->foo = foo;
            tr->position.x = std::clamp(tr->position.x, (float)foo->l, (float)foo->r);
            tr->position.y = foo->get_y(tr->position.x).value();
        }
    }
    return state;
}

int summon_follow(Summon *sum, Move *mv, Transform *tr)
{
    int state = sum->state;
    if (World::registry->valid(sum->owner))
    {
        auto o_tr = World::registry->try_get<Transform>(sum->owner);
        auto s_tr = tr;
        if (std::abs(o_tr->position.x - s_tr->position.x) > 200)
        {
            if (o_tr->position.x > s_tr->position.x)
            {
                mv->hspeed = mv->hspeed_max.value();
            }
            else
            {
                mv->hspeed = mv->hspeed_min.value();
            }
            state = Summon::State::MOVE;
        }
        else
        {
            mv->hspeed = 0;
            state = Summon::State::STAND;
        }
    }
    return state;
}

int summon_action(Summon *sum, int state, int new_state)
{
    if (state != new_state)
    {
        int action = new_state;
        switch (action)
        {
        case Summon::State::STAND:
        {
            sum->index = u"stand";
        }
        break;
        case Summon::State::MOVE:
        {
            sum->index = u"move";
        }
        break;
        case Summon::State::ATTACK:
        {
            sum->index = u"attack1";
        }
        break;
        case Summon::State::DIE:
        {
            sum->index = u"die";
        }
        break;
        }
        sum->a[sum->index].anim_index = 0;
        sum->a[sum->index].anim_time = 0;
    }
    return new_state;
}
