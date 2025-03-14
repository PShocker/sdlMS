#include "PetStateMachine.h"
#include "Commons/Commons.h"
#include "Move.h"
#include "Pick.h"

void pet_statemachine_run()
{
    auto view = World::registry->view<Pet>();
    for (auto ent : view)
    {
        pet_statemachine(ent, (float)Window::delta_time / 1000);
    }
}

void pet_statemachine(entt::entity ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto pet = World::registry->try_get<Pet>(ent);

    auto state = pet->state;
    switch (state)
    {
    case Pet::State::STAND:
    case Pet::State::MOVE:
    {
        pet_flip(mv, tr);
        pet_pick(ent);
        pet->state = pet_follow(ent);
        if (pet->state == Pet::State::STAND || pet->state == Pet::State::MOVE)
        {
            pet_move(ent, state, delta_time);
            if (pet_jump(pet, mv))
            {
                pet->state = Pet::State::JUMP;
            }
        }
    }
    break;
    case Pet::State::JUMP:
    {
        if (!pet_fall(ent, delta_time))
        {
            pet->state = Pet::State::STAND;
        }
    }
    break;
    case Pet::State::CLIMB:
    {
        if (!pet_climb(ent))
        {
            pet->state = Pet::State::JUMP;
        }
    }
    break;
    }
    pet_action(pet, state, pet->state);
}

void pet_flip(Move *mv, Transform *tr)
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

int pet_move(entt::entity ent, int state, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    move_move(mv, tr, 800, delta_time, false);
    return 0;
}

bool pet_fall(entt::entity ent, float delta_time)
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

bool pet_jump(Pet *pet, Move *mv)
{
    if (mv->hspeed == 0)
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
    else
    {
        // 向左移动
        fh = mv->foo->prev;
    }
    if (fh == nullptr || fh->k.has_value() == false)
    {
        // 有概率起跳
        int random = std::rand() % 20;
        if (random == 0)
        {
            pet->index = u"jump";
            pet->state = Pet::State::JUMP;

            mv->foo = nullptr;
            mv->vspeed = -420;
            return true;
        }
    }
    return false;
}

int pet_follow(entt::entity ent)
{
    auto pet = World::registry->try_get<Pet>(ent);
    if (World::registry->valid(pet->owner))
    {
        auto pet_mv = World::registry->try_get<Move>(ent);
        auto pet_tr = World::registry->try_get<Transform>(ent);
        // 首先判断是否需要瞬移
        auto owner_tr = World::registry->try_get<Transform>(pet->owner);
        auto owner_mv = World::registry->try_get<Move>(pet->owner);
        auto character = World::registry->try_get<Character>(pet->owner);
        if (character->action == Character::ACTION::LADDER || character->action == Character::ACTION::ROPE)
        {
            auto eff = World::registry->try_get<Effect>(ent);
            eff->effects.push_back({new Transform(pet_tr->position), AnimatedSprite(Effect::load(u"PetEff.img/" + pet->id + u"/warp"))});
            pet_tr->position = owner_tr->position;
            pet_tr->z = owner_tr->z + 1;
            eff->effects.push_back({nullptr, AnimatedSprite(Effect::load(u"PetEff.img/Basic/Teleport"))});
            return Pet::State::CLIMB;
        }
        else if (pet_tr->z != owner_tr->z - 3 ||
                 std::abs(owner_tr->position.x - pet_tr->position.x) >= 200 ||
                 std::abs(owner_tr->position.y - pet_tr->position.y) >= 200)
        {
            auto eff = World::registry->try_get<Effect>(ent);
            eff->effects.push_back({new Transform(pet_tr->position), AnimatedSprite(Effect::load(u"PetEff.img/" + pet->id + u"/warp"))});
            pet_mv->foo = owner_mv->foo;
            pet_mv->vspeed = 0;
            pet_mv->hspeed = 0;
            pet_tr->position = owner_tr->position;
            pet_tr->z = owner_tr->z - 3;
            eff->effects.push_back({nullptr, AnimatedSprite(Effect::load(u"PetEff.img/Basic/Teleport"))});
            return Pet::State::JUMP;
        }
        if (std::abs(owner_tr->position.x - pet_tr->position.x) >= 100)
        {
            pet_mv->hforce = owner_tr->position.x > pet_tr->position.x ? 1400 : -1400;
            return Pet::State::MOVE;
        }
        else
        {
            pet_mv->hforce = 0;
            return Pet::State::STAND;
        }
    }
    return 0;
}

bool pet_climb(entt::entity ent)
{
    auto pet = World::registry->try_get<Pet>(ent);
    if (World::registry->valid(pet->owner))
    {
        auto pet_mv = World::registry->try_get<Move>(ent);
        auto pet_tr = World::registry->try_get<Transform>(ent);
        auto owner_tr = World::registry->try_get<Transform>(pet->owner);
        auto owner_mv = World::registry->try_get<Move>(pet->owner);
        auto character = World::registry->try_get<Character>(pet->owner);

        if (character->action == Character::ACTION::LADDER || character->action == Character::ACTION::ROPE)
        {
            pet_tr->position = owner_tr->position;
        }
        else
        {
            // 脱离绳索
            pet_mv->vspeed = owner_mv->vspeed;
            pet_mv->hspeed = owner_mv->hspeed;
            auto eff = World::registry->try_get<Effect>(ent);
            eff->effects.push_back({new Transform(pet_tr->position), AnimatedSprite(Effect::load(u"PetEff.img/" + pet->id + u"/warp"))});
            pet_tr->position = owner_tr->position;
            pet_tr->z = owner_tr->z - 3;
            eff->effects.push_back({nullptr, AnimatedSprite(Effect::load(u"PetEff.img/Basic/Teleport"))});
            return false;
        }
    }
    return true;
}

void pet_pick(entt::entity ent)
{
    pick_drop(ent);
}

int pet_action(Pet *pet, int state, int new_state)
{
    if (state != new_state)
    {
        int action = new_state;
        switch (action)
        {
        case Pet::State::STAND:
        {
            pet->index = u"stand0";
        }
        break;
        case Pet::State::MOVE:
        {
            pet->index = u"move";
        }
        break;
        case Pet::State::JUMP:
        {
            pet->index = u"jump";
        }
        break;
        }
        pet->a[pet->index].anim_index = 0;
        pet->a[pet->index].anim_time = 0;
    }
    return new_state;
}