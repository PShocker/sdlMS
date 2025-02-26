#include "SummonStateMachine.h"
#include "Commons/Commons.h"
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
            sum->state = summon_move(ent, state, delta_time);
        }
    }
    break;
    case Summon::State::FLY:
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
            sum->state = summon_fly(ent, state, delta_time);
        }
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

int summon_move(entt::entity ent, int state, float delta_time)
{
    auto sum = World::registry->try_get<Summon>(ent);
    auto s_mv = World::registry->try_get<Move>(ent);
    auto s_tr = World::registry->try_get<Transform>(ent);
    // 跟随玩家
    if (World::registry->valid(sum->owner))
    {
        auto o_tr = World::registry->try_get<Transform>(sum->owner);
        // 先判断玩家周围是否存在怪物
        if (auto e = summon_attack(o_tr); World::registry->valid(e))
        {
            auto m_tr = World::registry->try_get<Transform>(e);
            if (m_tr->position.x > s_tr->position.x + 200)
            {
                s_mv->hspeed = s_mv->hspeed_max.value();
            }
            else if (m_tr->position.x < s_tr->position.x - 200)
            {
                s_mv->hspeed = s_mv->hspeed_min.value();
            }
        }
        else
        {
            state = summon_follow(ent);
        }
        auto foo = s_mv->foo;
        if (!move_move(s_mv, s_tr, 0, delta_time) && s_mv->foo == nullptr)
        {
            // 不掉落
            s_mv->foo = foo;
            s_tr->position.x = std::clamp(s_tr->position.x, (float)foo->l, (float)foo->r);
            s_tr->position.y = foo->get_y(s_tr->position.x).value();
        }
    }
    return state;
}

int summon_fly(entt::entity ent, int state, float delta_time)
{
    auto sum = World::registry->try_get<Summon>(ent);
    auto s_mv = World::registry->try_get<Move>(ent);
    auto s_tr = World::registry->try_get<Transform>(ent);
    // 跟随玩家
    if (World::registry->valid(sum->owner))
    {
        auto o_tr = World::registry->try_get<Transform>(sum->owner);
        // 先判断玩家周围是否存在怪物
        if (auto e = summon_attack(o_tr); World::registry->valid(e))
        {
            auto m_tr = World::registry->try_get<Transform>(e);
            if (m_tr->position.x > s_tr->position.x + 200)
            {
                s_mv->hspeed = s_mv->hspeed_max.value();
            }
            else if (m_tr->position.x < s_tr->position.x - 200)
            {
                s_mv->hspeed = s_mv->hspeed_min.value();
            }
            if (m_tr->position.y > s_tr->position.y + 200)
            {
                s_mv->vspeed = s_mv->hspeed_max.value();
            }
            else if (m_tr->position.y < s_tr->position.y - 200)
            {
                s_mv->vspeed = s_mv->hspeed_min.value();
            }
        }
        else
        {
            state = summon_follow(ent);
        }
        move_fall(s_mv, s_tr, delta_time, s_tr->z % LAYER_Z);
    }
    return state;
}

int summon_follow(entt::entity ent)
{
    auto sum = World::registry->try_get<Summon>(ent);
    auto s_mv = World::registry->try_get<Move>(ent);
    auto s_tr = World::registry->try_get<Transform>(ent);
    int state = sum->state;
    if (World::registry->valid(sum->owner))
    {
        auto o_tr = World::registry->try_get<Transform>(sum->owner);
        // 如果距离过远,则瞬移到身边
        if (s_tr->z != o_tr->z - 2 || distance(o_tr->position, s_tr->position) >= 600)
        {
            auto o_mv = World::registry->try_get<Move>(sum->owner);
            if (sum->state == Summon::State::FLY)
            {
                s_mv->vspeed = 0;
                s_mv->hspeed = 0;
                s_tr->position = o_tr->position;
                s_tr->z = o_tr->z - 2;
                auto eff = World::registry->try_get<Effect>(ent);
                eff->effects.push_back({nullptr, AnimatedSprite(Effect::load(u"BasicEff.img/Teleport"))});
                return Summon::State::FLY;
            }
            else if (o_mv->foo != nullptr)
            {
                s_mv->foo = o_mv->foo;
                s_mv->vspeed = 0;
                s_mv->hspeed = 0;
                s_tr->position = o_tr->position;
                s_tr->z = o_tr->z - 2;
                auto eff = World::registry->try_get<Effect>(ent);
                eff->effects.push_back({nullptr, AnimatedSprite(Effect::load(u"BasicEff.img/Teleport"))});
                return Summon::State::STAND;
            }
        }
        if (sum->state == Summon::State::FLY)
        {
            if (std::abs(o_tr->position.x - s_tr->position.x) > 200)
            {
                if (o_tr->position.x > s_tr->position.x)
                {
                    s_mv->hspeed = s_mv->hspeed_max.value();
                }
                else
                {
                    s_mv->hspeed = s_mv->hspeed_min.value();
                }
            }
            if (std::abs(o_tr->position.y - s_tr->position.y) > 50)
            {
                if (o_tr->position.y > s_tr->position.y)
                {
                    s_mv->vspeed = s_mv->hspeed_max.value();
                }
                else
                {
                    s_mv->vspeed = s_mv->hspeed_min.value();
                }
            }
        }
        else if (std::abs(o_tr->position.x - s_tr->position.x) > 200)
        {
            if (o_tr->position.x > s_tr->position.x)
            {
                s_mv->hspeed = s_mv->hspeed_max.value();
            }
            else
            {
                s_mv->hspeed = s_mv->hspeed_min.value();
            }
            state = Summon::State::MOVE;
        }
        else
        {
            s_mv->hspeed = 0;
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
