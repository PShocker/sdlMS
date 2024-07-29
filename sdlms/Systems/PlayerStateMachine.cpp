module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>
#include <variant>

module systems;

import components;
import commons;
import core;
import entities;

void player_statemachine_run()
{
    if (auto ent = Player::ent; World::registry->valid(ent))
    {
        player_cooldown(Window::delta_time);
        player_statemachine(&ent, (float)Window::delta_time / 1000);
    }
}

void player_statemachine(entt::entity *ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(*ent);
    auto tr = World::registry->try_get<Transform>(*ent);
    auto cha = World::registry->try_get<Character>(*ent);
    auto state = cha->state;

    switch (cha->state)
    {
    case Character::State::STAND:
    {
        player_flip(tr);
        if (player_climb(mv, tr, cha->state))
        {
            cha->state = Character::State::CLIMB;
            break;
        }
        if (player_prone(mv, tr, cha->state))
        {
            cha->state = Character::State::PRONE;
            break;
        }
        cha->state = player_walk(mv, tr, delta_time);
        if (player_jump(mv, cha, tr, cha->state))
        {
            cha->state = Character::State::JUMP;
        }
        cha->state = player_attack(mv, cha, tr, cha->state, ent);
    }
    break;
    case Character::State::WALK:
    {
        // normal status,can walk or jump
        player_flip(tr);
        if (player_climb(mv, tr, cha->state))
        {
            cha->state = Character::State::CLIMB;
            break;
        }
        if (player_prone(mv, tr, cha->state))
        {
            cha->state = Character::State::PRONE;
            break;
        }
        cha->state = player_walk(mv, tr, delta_time);
        if (player_jump(mv, cha, tr, cha->state))
        {
            cha->state = Character::State::JUMP;
        }
        cha->state = player_attack(mv, cha, tr, cha->state, ent);
    }
    break;
    case Character::State::JUMP:
    {
        player_flip(tr);
        if (!player_fall(mv, tr, ent, delta_time))
        {
            cha->state = Character::State::STAND;
            player_statemachine(ent, 0);
        }
        if (player_climb(mv, tr, cha->state))
        {
            cha->state = Character::State::CLIMB;
            break;
        }
        cha->state = player_attack(mv, cha, tr, cha->state, ent);
    }
    break;
    case Character::State::ATTACK:
    {
        if (!player_attacking(mv, cha, tr, ent, delta_time))
        {
            player_statemachine(ent, 0);
        }
    }
    break;
    case Character::State::CLIMB:
    {
        player_flip(tr);
        if (player_jump(mv, cha, tr, cha->state))
        {
            cha->state = Character::State::JUMP;
            break;
        }
        cha->state = player_climbing(cha, mv, tr, ent, delta_time);
    }
    break;
    case Character::State::PRONE:
    {
        player_flip(tr);
        if (player_jump(mv, cha, tr, cha->state))
        {
            cha->state = Character::State::JUMP;
            break;
        }
        if (!player_proning())
        {
            cha->state = Character::State::STAND;
            player_statemachine(ent, 0);
        }
    }
    break;
    default:
        break;
    }
    player_portal(ent);
    player_action(cha, state, cha->state, mv);
}

void player_flip(Transform *tr)
{
    if (Input::state[SDL_SCANCODE_RIGHT])
    {
        tr->flip = 1;
    }
    else if (Input::state[SDL_SCANCODE_LEFT])
    {
        tr->flip = 0;
    }
}

int player_walk(Move *mv, Transform *tr, float delta_time)
{
    auto state = Character::State::WALK;

    if (Input::state[SDL_SCANCODE_RIGHT])
    {
        mv->hforce = 1400;
    }
    else if (Input::state[SDL_SCANCODE_LEFT])
    {
        mv->hforce = -1400;
    }
    else
    {
        state = Character::State::STAND;

        mv->hforce = 0;
        // 如果没有左右的输入并且速度为0,则可以直接return提高性能
        if (mv->hspeed == 0)
        {
            return state;
        }
    }

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
    if (auto new_x = player_border_limit(mv, x); x != new_x)
    {
        x = new_x;
        mv->hspeed = 0;
    }
    auto y = tr->position.y;

    // 人物在fh移动的函数
    auto walk_fh = [&x, &y, &tr, &mv, &state](FootHold *next_fh) -> bool
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
            state = Character::State::JUMP;
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
                state = Character::State::JUMP;
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

bool player_fall(Move *mv, Transform *tr, entt::entity *ent, float delta_time)
{
    if (Input::state[SDL_SCANCODE_RIGHT])
    {
        mv->hspeed += 0.3f;
    }
    else if (Input::state[SDL_SCANCODE_LEFT])
    {
        mv->hspeed -= 0.3f;
    }

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

    if (auto new_x = player_border_limit(mv, new_pos.x); new_pos.x != new_x)
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
    if (mv->vspeed >= 0 && player_foothold_cooldown <= 0)
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
                else
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

bool player_jump(Move *mv, Character *cha, Transform *tr, int state)
{
    if (Input::state[SDL_SCANCODE_LALT])
    {
        if (mv->foo)
        {
            if (state == Character::State::PRONE)
            {
                if (player_down_jump(mv, tr))
                {
                    mv->vspeed = -100;
                    mv->page = mv->foo->page;
                    mv->zmass = mv->foo->zmass;
                    mv->foo = nullptr;
                    mv->lr = nullptr;
                    player_foothold_cooldown = 120;
                    return true;
                }
            }
            else
            {
                mv->vspeed = -555;
                mv->page = mv->foo->page;
                mv->zmass = mv->foo->zmass;
                mv->foo = nullptr;
                mv->lr = nullptr;
                return true;
            }
        }
        else if (mv->lr)
        {
            if (!Input::state[SDL_SCANCODE_UP] && (Input::state[SDL_SCANCODE_RIGHT] || Input::state[SDL_SCANCODE_LEFT]))
            {
                cha->animate = true;

                mv->vspeed = -310;
                mv->lr = nullptr;
                if (Input::state[SDL_SCANCODE_RIGHT])
                {
                    mv->hspeed = 140;
                }
                else if (Input::state[SDL_SCANCODE_LEFT])
                {
                    mv->hspeed = -140;
                }
                return true;
            }
        }
    }
    return false;
}

bool player_down_jump(Move *mv, Transform *tr)
{
    auto view = World::registry->view<FootHold>();
    for (auto &e : view)
    {
        auto fh = &view.get<FootHold>(e);
        if (fh->get_y(tr->position.x).has_value() &&
            fh->get_y(tr->position.x).value() < tr->position.y + 600 &&
            fh->get_y(tr->position.x).value() > tr->position.y &&
            fh != mv->foo)
        {
            return true;
        }
    }
    return false;
}

int player_attack(Move *mv, Character *cha, Transform *tr, int state, entt::entity *ent)
{
    if (Input::state[SDL_SCANCODE_LCTRL])
    {
        if (state != Character::State::JUMP)
        {
            mv->hspeed = 0;
        }
        // add afterimg
        player_attack_afterimage(ent);

        state = Character::State::ATTACK;
        cha->animated = false;
    }
    return state;
}

bool player_attacking(Move *mv, Character *cha, Transform *tr, entt::entity *ent, float delta_time)
{
    if (mv->foo == nullptr)
    {
        if (player_fall(mv, tr, ent, delta_time))
        {
            // 空中
            if (cha->animated)
            {
                // 攻击动画完成
                cha->state = Character::State::JUMP;
                return false;
            }
        }
        else
        {
            // 落地
            if (cha->animated)
            {
                // 攻击动画完成
                cha->state = Character::State::STAND;
                return false;
            }
            mv->hspeed = 0;
        }
    }
    else
    {
        // 地面
        if (cha->animated)
        {
            // 攻击动画完成
            cha->state = Character::State::STAND;
            return false;
        }
    }
    return true;
}

bool player_attack_afterimage(entt::entity *ent)
{
    World::registry->emplace_or_replace<AfterImage>(*ent);
    auto pski = &World::registry->emplace_or_replace<PlayerSkill>(*ent, u"1311006");
    World::registry->emplace_or_replace<Attack>(*ent, pski);
    return true;
}

bool player_climb(Move *mv, Transform *tr, int state)
{
    if (Input::state[SDL_SCANCODE_UP] || Input::state[SDL_SCANCODE_DOWN])
    {
        auto view = World::registry->view<LadderRope>();
        for (auto &e : view)
        {
            auto lr = &view.get<LadderRope>(e);

            // 判断x坐标是否在梯子范围内
            if (tr->position.x >= lr->x - 15 && tr->position.x <= lr->x + 15)
            {
                int t = 0;
                int b = lr->b + 5;
                if (mv->foo)
                {
                    if (Input::state[SDL_SCANCODE_DOWN])
                    {
                        t = lr->t - 10;
                        b = lr->t;
                    }
                    else
                    {
                        t = lr->b;
                    }
                }
                else
                {
                    t = lr->t;
                }
                if (!(mv->foo == nullptr && Input::state[SDL_SCANCODE_DOWN]))
                {
                    if (tr->position.y >= t && tr->position.y <= b)
                    {
                        // 爬到梯子
                        mv->lr = lr;
                        mv->zmass = 0;
                        mv->hspeed = 0;
                        mv->vspeed = 0;
                        mv->foo = nullptr;

                        tr->position.x = lr->x;
                        tr->position.y = std::clamp(tr->position.y, (float)lr->t, (float)lr->b);

                        tr->z = lr->page * LAYER_Z + CHARACTER_Z;
                        World::zindex = true;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

int player_climbing(Character *cha, Move *mv, Transform *tr, entt::entity *ent, float delta_time)
{
    auto state = Character::State::CLIMB;
    if (Input::state[SDL_SCANCODE_DOWN])
    {
        mv->vspeed = 100;
        cha->animate = true;
    }
    else if (Input::state[SDL_SCANCODE_UP])
    {
        mv->vspeed = -100;
        cha->animate = true;
    }
    else
    {
        mv->vspeed = 0;
        cha->animate = false;
    }

    auto d_y = mv->vspeed * delta_time;
    auto y = d_y + tr->position.y;

    if (y >= mv->lr->t && y <= mv->lr->b)
    {
        tr->position.y = y;
    }
    else if (y < mv->lr->t)
    {
        if (mv->lr->uf == 1)
        {
            tr->position.y = mv->lr->t - 5;
            mv->vspeed = 0;
            state = Character::State::JUMP;
            cha->animate = true;
        }
        else
        {
            cha->animate = false;
        }
    }
    else if (y > mv->lr->b)
    {
        tr->position.y = mv->lr->b;
        mv->vspeed = 0;
        state = Character::State::JUMP;
        cha->animate = true;
    }
    return state;
}

bool player_prone(Move *mv, Transform *tr, int state)
{
    if (Input::state[SDL_SCANCODE_DOWN])
    {
        if (mv->foo)
        {
            mv->hspeed = 0;
            return true;
        }
    }
    return false;
}

bool player_proning()
{
    if (Input::state[SDL_SCANCODE_DOWN])
    {
        return true;
    }
    return false;
}

float player_border_limit(Move *mv, float x)
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

void player_action(Character *cha, int state, int new_state, Move *mv)
{
    if (state != new_state)
    {
        int action = new_state;
        switch (action)
        {
        case Character::State::STAND:
            action = Character::ACTION::STAND1;
            break;
        case Character::State::WALK:
            action = Character::ACTION::WALK1;
            break;
        case Character::State::JUMP:
            action = Character::ACTION::JUMP;
            break;
        case Character::State::ATTACK:
        {
            int random = std::rand() % 3;
            switch (random)
            {
            case 0:
                action = Character::ACTION::SWINGO1;
                break;
            case 1:
                action = Character::ACTION::SWINGO2;
                break;
            case 2:
                action = Character::ACTION::SWINGO3;
                break;
            default:
                break;
            }
        }
        break;
        case Character::State::CLIMB:
        {
            switch (mv->lr->l)
            {
            case 1:
                action = Character::ACTION::LADDER;
                break;
            default:
                action = Character::ACTION::ROPE;
                break;
            }
        }
        break;
        case Character::State::PRONE:
            action = Character::ACTION::PRONE;
            break;
        }
        cha->action_index = 0;
        cha->action_time = 0;
        cha->action = action;
    }
}

void player_cooldown(int delta_time)
{
    if (player_foothold_cooldown > 0)
    {
        player_foothold_cooldown -= delta_time;
    }
    if (player_portal_cooldown > 0)
    {
        player_portal_cooldown -= delta_time;
    }
}

void player_portal(entt::entity *ent)
{
    if (player_portal_cooldown <= 0)
    {
        auto tr = World::registry->try_get<Transform>(*ent);
        auto view = World::registry->view<Portal>();
        for (auto &e : view)
        {
            auto por = &view.get<Portal>(e);
            if (por->tm != 999999999)
            {
                if (por->pt == 3 || Input::state[SDL_SCANCODE_UP])
                {
                    auto player_pos = tr->position;
                    auto por_tr = World::registry->try_get<Transform>(e);
                    auto por_x = por_tr->position.x;
                    auto por_y = por_tr->position.y;
                    if (player_pos.x == std::clamp(player_pos.x, por_x - 20, por_x + 20) &&
                        player_pos.y == std::clamp(player_pos.y, por_y - 50, por_y + 50))
                    {
                        if (por->tm != Map::id)
                        {
                            // need to change map
                            World::TransPort::id = por->tm;
                            World::TransPort::tn = std::get<std::u16string>(por->tn);
                        }
                        else
                        {
                            auto position = std::get<SDL_FPoint>(por->tn);
                            tr->position.x = position.x;
                            tr->position.y = position.y - 5;
                            auto cha = World::registry->try_get<Character>(Player::ent);
                            cha->state = Character::State::JUMP;
                        }
                        player_portal_cooldown = 800;
                        break;
                    }
                }
            }
        }
    }
}
