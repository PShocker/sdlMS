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
        summon_flip(mv, tr);
        sum->state = summon_follow(ent);
        if (sum->state == Summon::State::STAND || sum->state == Summon::State::MOVE)
        {
            if (summon_jump(sum, tr, mv))
            {
                sum->state = Summon::State::JUMP;
                break;
            }
            if (summon_move(ent, state, delta_time))
            {
                sum->state = Summon::State::JUMP;
                break;
            }
            if (summon_attack_action(ent, summon_attack(tr)))
            {
                break;
            }
        }
    }
    break;
    case Summon::State::JUMP:
    {
        summon_flip(mv, tr);
        sum->state = summon_follow(ent);
        if (!summon_fall(ent, delta_time))
        {
            sum->state = Summon::State::STAND;
        }
    }
    break;
    case Summon::State::FLY:
    {
        summon_flip(mv, tr);
        summon_follow(ent);
        summon_fly(ent, state, delta_time);
        if (summon_attack_action(ent, summon_attack(tr)))
        {
            break;
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
    for (auto ent : World::registry->view<Mob>())
    {
        auto mob = World::registry->try_get<Mob>(ent);
        if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
        {
            auto mob_tr = World::registry->try_get<Transform>(ent);
            if (std::abs(mob_tr->position.x - tr->position.x) <= 350 &&
                std::abs(mob_tr->position.y - tr->position.y) <= 35)
            {
                return ent;
            }
        }
    }
    return entt::null;
}

bool summon_attack_action(entt::entity ent, entt::entity target)
{
    if (World::registry->valid(target))
    {
        // 转向
        auto mob_tr = World::registry->try_get<Transform>(target);
        auto summon_tr = World::registry->try_get<Transform>(ent);
        summon_tr->flip = mob_tr->position.x > summon_tr->position.x ? 1 : 0;

        auto summon_mv = World::registry->try_get<Move>(ent);
        summon_mv->hspeed = 0;
        summon_mv->vspeed = 0;

        auto sum = World::registry->try_get<Summon>(ent);
        sum->state = Summon::State::ATTACK;
        return true;
    }
    return false;
}

bool summon_move(entt::entity ent, int state, float delta_time)
{
    auto sum = World::registry->try_get<Summon>(ent);
    auto summon_mv = World::registry->try_get<Move>(ent);
    auto summon_tr = World::registry->try_get<Transform>(ent);

    if (World::registry->valid(sum->owner))
    {
        // 跟随玩家
        if (auto e = summon_attack(World::registry->try_get<Transform>(sum->owner)); World::registry->valid(e))
        {
            summon_set_hspeed(ent, e, 0, 1);
        }
        move_move(summon_mv, summon_tr, 0, delta_time);
    }
    return summon_mv->foo == nullptr;
}

bool summon_fall(entt::entity ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    if (mv->foo)
    {
        return false;
    }
    else
    {
        // 默认重力为2000
        mv->vspeed += delta_time * 2000;
        // 往人物方向运动
        auto sum = World::registry->try_get<Summon>(ent);
        auto owner_tr = World::registry->try_get<Transform>(sum->owner);
        if (std::abs(owner_tr->position.x - tr->position.x) > 5)
        {
            mv->hspeed += owner_tr->position.x >= tr->position.x ? 0.1f : -0.1f;
        }
        if (move_fall(mv, tr, delta_time, tr->z % LAYER_Z))
        {
            return true;
        }
        else
        {
            // 落地
            return false;
        }
    }
}

void summon_fly(entt::entity ent, int state, float delta_time)
{
    auto sum = World::registry->try_get<Summon>(ent);
    auto summon_mv = World::registry->try_get<Move>(ent);
    auto summon_tr = World::registry->try_get<Transform>(ent);
    // 跟随玩家
    if (World::registry->valid(sum->owner))
    {
        if (auto e = summon_attack(World::registry->try_get<Transform>(sum->owner)); World::registry->valid(e))
        {
            summon_set_hspeed(ent, e, 0, 1);
            summon_set_vspeed(ent, e, 0, 3);
        }
        move_fall(summon_mv, summon_tr, delta_time, summon_tr->z % LAYER_Z, false, false);
    }
}

int summon_follow(entt::entity ent)
{
    auto sum = World::registry->try_get<Summon>(ent);
    auto summon_mv = World::registry->try_get<Move>(ent);
    auto summon_tr = World::registry->try_get<Transform>(ent);
    int state = sum->state;
    if (World::registry->valid(sum->owner))
    {
        auto owner_tr = World::registry->try_get<Transform>(sum->owner);
        // 如果距离过远,则瞬移到身边
        if (std::abs(owner_tr->position.x - summon_tr->position.x) >= 600 ||
            std::abs(owner_tr->position.y - summon_tr->position.y) >= 300)
        {
            summon_mv->vspeed = 0;
            summon_mv->hspeed = 0;
            summon_mv->foo = nullptr;
            summon_tr->position = owner_tr->position + SDL_FPoint{0, -5};
            auto eff = World::registry->try_get<Effect>(ent);
            eff->effects.emplace(0, Effect::Wrap{std::nullopt, AnimatedSprite(Effect::load(u"BasicEff.img/Teleport"))});
            if (sum->state == Summon::State::FLY)
            {
                return Summon::State::FLY;
            }
            else
            {
                return Summon::State::JUMP;
            }
        }
        if (sum->state == Summon::State::FLY)
        {
            summon_set_hspeed(ent, sum->owner, 70, 1);
            summon_set_vspeed(ent, sum->owner, 10, 3);
        }
        else if (summon_mv->foo)
        {
            summon_set_hspeed(ent, sum->owner, 70, 1);
            if (summon_mv->hspeed == 0)
            {
                state = Summon::State::STAND;
            }
            else
            {
                state = Summon::State::MOVE;
            }
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

bool summon_jump(Summon *sum, Transform *tr, Move *mv)
{
    if (mv->foo == nullptr)
    {
        return false;
    }
    FootHold *fh = nullptr;
    // 判断移动方向
    if (mv->hspeed > 0)
    {
        // 向右移动
        fh = mv->foo->next;
    }
    else if (mv->hspeed < 0)
    {
        // 向左移动
        fh = mv->foo->prev;
    }
    else
    {
        return false;
    }
    if (fh == nullptr || fh->k.has_value() == false && tr->position.y >= fh->b)
    {
        // 有概率起跳
        int random = std::rand() % 8;
        if (random == 0)
        {
            sum->index = u"jump";
            sum->state = Summon::State::JUMP;

            mv->foo = nullptr;
            mv->vspeed = -600;
            return true;
        }
    }
    return false;
}

void summon_set_hspeed(entt::entity ent, entt::entity target, float distance, float speed)
{
    auto target_tr = World::registry->try_get<Transform>(target);
    auto summon_tr = World::registry->try_get<Transform>(ent);
    auto summon_mv = World::registry->try_get<Move>(ent);
    if (std::abs(target_tr->position.x - summon_tr->position.x) >= distance)
    {
        auto sum = World::registry->try_get<Summon>(ent);
        auto owner_mv = World::registry->try_get<Move>(sum->owner);
        speed = speed * std::abs(owner_mv->hspeed_max.value()) / 125.0f;
        if (target_tr->position.x > summon_tr->position.x)
        {
            summon_mv->hspeed = (target_tr->position.x - summon_tr->position.x - distance) * speed;
            summon_mv->hspeed = std::min(summon_mv->hspeed, owner_mv->hspeed_max.value());
        }
        else
        {
            summon_mv->hspeed = (target_tr->position.x - summon_tr->position.x + distance) * speed;
            summon_mv->hspeed = std::max(summon_mv->hspeed, owner_mv->hspeed_min.value());
        }
        summon_mv->hspeed_max = std::abs(summon_mv->hspeed);
        summon_mv->hspeed_min = -std::abs(summon_mv->hspeed);
    }
}

void summon_set_vspeed(entt::entity ent, entt::entity target, float distance, float speed)
{
    auto target_tr = World::registry->try_get<Transform>(target);
    auto summon_tr = World::registry->try_get<Transform>(ent);
    auto summon_mv = World::registry->try_get<Move>(ent);
    if (std::abs(target_tr->position.y - summon_tr->position.y) >= distance)
    {
        if (target_tr->position.y > summon_tr->position.y)
        {
            summon_mv->vspeed = (target_tr->position.y - summon_tr->position.y - distance) * speed;
        }
        else
        {
            summon_mv->vspeed = (target_tr->position.y - summon_tr->position.y + distance) * speed;
        }
    }
}
