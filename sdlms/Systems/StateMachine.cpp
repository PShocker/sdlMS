module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>

module systems;

import components;
import commons;
import core;
import entities;

void statemachine_run()
{
    if (auto ent = Player::ent; World::registry.valid(ent))
    {
        auto delta_time = (float)Window::delta_time / 1000;
        player_statemachine(&ent, delta_time);
    }
}

void player_statemachine(entt::entity *ent, float delta_time)
{
    auto mv = World::registry.try_get<Move>(*ent);
    auto tr = World::registry.try_get<Transform>(*ent);
    auto cha = World::registry.try_get<Character>(*ent);
    auto state = cha->state;

    auto foo = mv->foo;
    switch (cha->state)
    {
    case Character::State::STAND:
    {
        player_flip(tr);
        cha->state = player_walk(mv, tr, foo, delta_time);
        cha->state = player_jump(mv, tr, cha->state);
        cha->state = player_attack(mv, cha, tr, cha->state, ent);
    }
    break;
    case Character::State::WALK:
    {
        // normal status,can walk or jump
        player_flip(tr);
        cha->state = player_walk(mv, tr, foo, delta_time);
        cha->state = player_jump(mv, tr, cha->state);
        cha->state = player_attack(mv, cha, tr, cha->state, ent);
    }
    break;
    case Character::State::JUMP:
    {
        player_flip(tr);
        if (!player_fall(mv, tr, foo, ent, delta_time))
        {
            cha->state = Character::State::STAND;
            player_statemachine(ent, 0);
        }
        cha->state = player_attack(mv, cha, tr, cha->state, ent);
    }
    break;
    case Character::State::ATTACK:
    {
        if (!player_attacking(mv, cha, tr, foo, ent, delta_time))
        {
            player_statemachine(ent, 0);
        }
    }
    break;
    default:
        break;
    }
    player_action(cha, state, cha->state);
}

void player_flip(Transform *tr)
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

int player_walk(Move *mv, Transform *tr, FootHold *foo, float delta_time)
{
    auto state = Character::State::WALK;

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
    auto y = tr->position.y;

    auto &fhs = FootHold::fhs;

    // 人物在fh移动的函数
    auto walk_fh = [&fhs, &foo, &x, &y, &tr, &mv, &state](int next_fh) -> bool
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

                mv->foo = nullptr;
                state = Character::State::JUMP;
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
            return state;
        }
    }
    // 往右走
    while (x > foo->r)
    {
        int next_fh = std::abs(foo->next);
        if (walk_fh(next_fh) == false)
        {
            return state;
        }
    }
    // 地面上
    tr->position.x = x;
    tr->position.y = foo->get_y(x).value();
    return state;
}

bool player_fall(Move *mv, Transform *tr, FootHold *foo, entt::entity *ent, float delta_time)
{
    if (Input::is_key_held(SDLK_RIGHT))
    {
        mv->hspeed += 0.3f;
    }
    else if (Input::is_key_held(SDLK_LEFT))
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
                    mv->hspeed = 0;
                    tr->position = new_pos;
                    // switch z
                    if ((foo != nullptr && foo->page != fh->page) || foo == nullptr)
                    {
                        tr->z = fh->page * LAYER_Z + CHARACTER_Z;
                        World::zindex();
                    }
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

int player_jump(Move *mv, Transform *tr, int state)
{
    if (Input::is_key_held(SDLK_LALT))
    {
        mv->vspeed = -555;

        mv->foo = nullptr;
        state = Character::State::JUMP;
    }
    return state;
}

int player_attack(Move *mv, Character *cha, Transform *tr, int state, entt::entity *ent)
{
    if (Input::is_key_held(SDLK_LCTRL))
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

bool player_attacking(Move *mv, Character *cha, Transform *tr, FootHold *foo, entt::entity *ent, float delta_time)
{
    if (mv->foo == nullptr)
    {
        if (player_fall(mv, tr, foo, ent, delta_time))
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
    World::registry.emplace_or_replace<AfterImage>(*ent);
    return true;
}

void player_action(Character *cha, int state, int new_state)
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
        }

        cha->action_index = 0;
        cha->action_time = 0;
        cha->action = action;
    }
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