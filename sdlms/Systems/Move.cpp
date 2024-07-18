module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

module systems;

import components;
import core;
import commons;

void move_run()
{
    auto view = World::registry.view<Move>();
    for (auto &ent : view)
    {
        auto mv = &view.get<Move>(ent);
        if (World::registry.any_of<Player>(ent))
        {
            auto tr = World::registry.try_get<Transform>(ent);
            auto cha = World::registry.try_get<Character>(ent);
            move_player(mv, tr, cha);
        }
    }
}

void move_player(Move *mv, Transform *tr, Character *cha)
{
    move_play_flip(tr);
    switch (cha->status)
    {
    case Character::Status::NONE:
    {
        // normal status,can walk or jump
    }
    break;
    case Character::Status::JUMP:
    {
    }
    break;
    default:
        break;
    }
}

void move_play_flip(Transform *tr)
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

void move_play_walk(Move *mv, Transform *tr)
{
}

void move_play_fall(Move *mv, Transform *tr)
{
    if (Input::is_key_held(SDLK_RIGHT))
    {
        mv->hspeed += 0.3f;
    }
    else if (Input::is_key_held(SDLK_LEFT))
    {
        mv->hspeed -= 0.3f;
    }

    auto delta_time = Window::delta_time;

    // 默认重力为2000
    mv->vspeed += delta_time * 2000;
    if (mv->vspeed_limit.has_value())
    {
        auto min = mv->vspeed_limit.value().x;
        auto max = mv->vspeed_limit.value().y;
        mv->vspeed = std::clamp(mv->vspeed, min, max);
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
}
