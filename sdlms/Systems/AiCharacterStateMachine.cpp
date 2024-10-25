module;

#include "entt/entt.hpp"

module systems;

import core;
import components;
import commons;
import entities;

void aicharacter_statemachine_run()
{
    auto view = World::registry->view<AiCharacter>();
    for (auto ent : view)
    {
        aicharacter_cooldown(World::registry->try_get<Character>(ent), Window::delta_time);
        if (aicharacter_hit(World::registry->try_get<Hit>(ent), &ent))
        {
            return;
        }
        aicharacter_statemachine(&ent, (float)Window::delta_time / 1000);
    }
}

void aicharacter_statemachine(entt::entity *ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(*ent);
    auto tr = World::registry->try_get<Transform>(*ent);
    auto cha = World::registry->try_get<Character>(*ent);
    auto state = cha->state;

    switch (cha->state)
    {
    case Character::State::JUMP:
    {
        if (!aicharacter_fall(mv, tr, delta_time))
        {
            cha->state = Character::State::STAND;
        }
    }
    break;
    case Character::State::DIE:
    {
        cha->invincible_cooldown = 2000;
        return;
    }
    break;
    }
    aicharacter_action(cha, state, cha->state, mv, ent);
}

bool aicharacter_fall(Move *mv, Transform *tr, float delta_time)
{
    // 默认重力为2000
    mv->vspeed += delta_time * 2000;

    return move_fall(mv, tr, delta_time, MOB_Z);
}

void aicharacter_action(Character *cha, int state, int new_state, Move *mv, entt::entity *ent)
{
    if (state != new_state)
    {
        int action = new_state;
        switch (action)
        {
        case Character::State::STAND:
        {
            auto weaponinfo = World::registry->try_get<WeaponInfo>(*ent);
            if (weaponinfo != nullptr && weaponinfo->stand1 == false)
            {
                action = Character::ACTION::STAND2;
            }
            else
            {
                action = Character::ACTION::STAND1;
            }
        }
        break;
        case Character::State::JUMP:
            action = Character::ACTION::JUMP;
            break;
        }

        cha->action_frame = 0;
        cha->action_index = 0;
        cha->action_time = 0;
        if (action != -1)
        {
            cha->action = action;
            cha->action_str = Character::type_map2.at(action);
        }
    }
}

bool aicharacter_hit(Hit *hit, entt::entity *ent)
{
    if (hit->damage > 0)
    {
        auto mv = World::registry->try_get<Move>(*ent);
        auto tr = World::registry->try_get<Transform>(*ent);
        auto cha = World::registry->try_get<Character>(*ent);
        auto count = hit->count;
        cha->hp -= hit->damage;
        cha->invincible_cooldown = 2000;
        hit->damage = 0;
        hit->count = 0;

        for (int i = 0; i < count; i++)
        {
            // 被攻击音效
            if (hit->souw)
            {
                Sound::push(hit->souw, i * 140);
            }
        }

        if (cha->hp > 0)
        {
            if (mv->foo && cha->action != Character::ACTION::PRONESTAB)
            {
                mv->vspeed = -320;
                auto hit_x = hit->x;
                auto cha_x = tr->position.x;
                if (cha_x < hit_x)
                {
                    mv->hspeed = -110;
                }
                else
                {
                    mv->hspeed = 110;
                }
                mv->foo = nullptr;
            }
            if (cha->state == Character::State::STAND || cha->state == Character::State::WALK || cha->state == Character::State::ALERT || cha->state == Character::State::PRONE)
            {
                cha->state = Character::State::JUMP;
                cha->action_index = 0;
                cha->action_time = 0;
                cha->action_frame = 0;
                cha->action = Character::ACTION::JUMP;
                cha->action_str = u"jump";
            }
        }
        else
        {
            cha->state = Character::State::DIE;
            cha->action_index = 0;
            cha->action_time = 0;
            cha->action_frame = 0;
            cha->action = Character::ACTION::DEAD;
            cha->action_str = u"dead";

            if (!mv->foo)
            {
                auto view = World::registry->view<FootHold>();
                std::optional<float> y = std::nullopt;
                for (auto &e : view)
                {
                    auto fh = &view.get<FootHold>(e);
                    if (fh->get_y(tr->position.x).has_value() && fh->get_y(tr->position.x).value() > tr->position.y)
                    {
                        if (!y.has_value())
                        {
                            y = (float)fh->get_y(tr->position.x).value();
                        }
                        else
                        {
                            y = std::min(y.value(), (float)fh->get_y(tr->position.x).value());
                        }
                    }
                }
                if (y.has_value())
                {
                    tr->position.y = y.value();
                }
            }
            auto &tomb = World::registry->emplace_or_replace<Tomb>(*ent);
            tomb.f.position = tr->position;
            tomb.f.position.y -= 200;
            tomb.l.position = tr->position;

            aicharacter_drop(cha, tr);
        }
        return true;
    }
    return false;
}

void aicharacter_cooldown(Character *cha, int delta_time)
{
    if (cha->invincible_cooldown > 0)
    {
        cha->invincible_cooldown -= delta_time;
    }
}

void aicharacter_drop(Character *cha, Transform *tr)
{
    std::vector<DropInfo> drops;
    drops.push_back({{u"01382014", 1}});
    load_drops(&drops, tr->position.x, tr->position.y, tr->z / LAYER_Z);
    World::zindex = true;
    Sound::push(Sound(u"Game.img/DropItem"));
}
