#include "PlayerStateMachine.h"
#include "Move.h"
#include "Hit.h"
#include "entt/entt.hpp"
#include "Commons/Commons.h"
#include "PlayerSkill/PlayerSkill.h"
#include "Core/Core.h"
#include <SDL3/SDL.h>
#include <optional>

void player_statemachine_run()
{
    if (auto ent = Player::ent; World::registry->valid(ent))
    {
        player_cooldown(Window::delta_time);
        if (player_hit(World::registry->try_get<Hit>(ent), ent))
        {
            return;
        }
        player_statemachine(ent, (float)Window::delta_time / 1000);
    }
}

void player_statemachine(entt::entity ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    switch (cha->state)
    {
    case Character::State::ALERT:
    case Character::State::STAND:
    case Character::State::WALK:
    {
        player_flip(tr);
        player_pick_drop(cha, tr);
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
        if (player_skill(mv, cha, tr, cha->state, ent))
        {
            cha->state = Character::State::SKILL;
            break;
        }
        if (player_sit(mv, cha->state))
        {
            cha->state = Character::State::SIT;
            break;
        }
        cha->state = player_attack(mv, cha, tr, cha->state, ent);
    }
    break;
    case Character::State::JUMP:
    {
        player_flip(tr);
        if (!player_fall(mv, tr, delta_time))
        {
            cha->state = Character::State::STAND;
            player_statemachine(ent, 0);
        }
        if (player_climb(mv, tr, cha->state))
        {
            cha->state = Character::State::CLIMB;
            break;
        }
        if (player_skill(mv, cha, tr, cha->state, ent))
        {
            cha->state = Character::State::SKILL;
            break;
        }
        if (player_double_jump(mv, tr, ent))
        {
            break;
        }
        cha->state = player_attack(mv, cha, tr, cha->state, ent);
    }
    break;
    case Character::State::ATTACK:
    {
        if (!player_animating(mv, cha, tr, ent, delta_time))
        {
            player_statemachine(ent, 0);
        }
    }
    break;
    case Character::State::SKILL:
    {
        if (!player_skilling(mv, cha, tr, ent, delta_time))
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
        if (player_skill(mv, cha, tr, cha->state, ent))
        {
            cha->state = Character::State::SKILL;
            break;
        }
        cha->state = player_climbing(cha, mv, tr, cha->state, ent, delta_time);
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
        if (player_skill(mv, cha, tr, cha->state, ent))
        {
            cha->state = Character::State::SKILL;
            break;
        }
        if (!player_proning())
        {
            cha->state = Character::State::STAND;
            player_statemachine(ent, 0);
            break;
        }
        cha->state = player_attack(mv, cha, tr, cha->state, ent);
    }
    break;
    case Character::State::DIE:
    {
        cha->invincible_cooldown = 2000;
        return;
    }
    break;
    case Character::State::SIT:
    {
        if (!player_sit(mv, cha->state))
        {
            cha->state = Character::State::STAND;
        }
    }
    break;
    default:
        break;
    }
    player_portal(mv, ent);
    player_face(cha);
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
        if (player_alert_cooldown > 0)
        {
            state = Character::State::ALERT;
        }
        else
        {
            state = Character::State::STAND;
        }
        mv->hforce = 0;
        // 如果没有左右的输入并且速度为0,则可以直接return提高性能
        if (mv->hspeed == 0)
        {
            return state;
        }
    }
    if (move_move(mv, tr, 800, delta_time))
    {
        return state;
    }
    else if (mv->foo)
    {
        return Character::State::WALK;
    }
    else
    {
        return Character::State::JUMP;
    }
}

bool player_fall(Move *mv, Transform *tr, float delta_time)
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
    auto vspeed = mv->vspeed + delta_time * 2000;
    if (mv->vspeed <= 0 && vspeed > 0)
    {
        player_fall_y = tr->position.y;
    }
    mv->vspeed = vspeed;
    auto r = move_fall(mv, tr, delta_time, CHARACTER_Z, player_foothold_cooldown <= 0);
    if (r == false)
    {
        auto cha = World::registry->try_get<Character>(Player::ent);
        if (cha->invincible_cooldown <= 0)
        {
            auto distance = tr->position.y - player_fall_y;
            if (distance >= 600)
            {
                AttackWarp atkw;
                atkw.damage = distance / 25;
                atkw.hit = nullptr;
                atkw.p = &tr->position;
                hit_effect(&atkw, std::nullopt, Player::ent, 1, nullptr);
            }
        }
    }
    return r;
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
                    SkillWarp::cooldowns[u"4111006"] = 500;

                    Sound::push(Sound(u"Game.img/Jump"));

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
                SkillWarp::cooldowns[u"4111006"] = 200;

                Sound::push(Sound(u"Game.img/Jump"));

                return true;
            }
        }
        else if (mv->lr)
        {
            if (!Input::state[SDL_SCANCODE_UP] && (Input::state[SDL_SCANCODE_RIGHT] || Input::state[SDL_SCANCODE_LEFT]))
            {
                cha->animate = true;

                mv->vspeed = -310;

                if (Input::state[SDL_SCANCODE_RIGHT])
                {
                    mv->hspeed = 140;
                }
                else if (Input::state[SDL_SCANCODE_LEFT])
                {
                    mv->hspeed = -140;
                }
                player_ladderrope_cooldown = 80;

                SkillWarp::cooldowns[u"4111006"] = 300;

                Sound::push(Sound(u"Game.img/Jump"));

                return true;
            }
        }
    }
    return false;
}

bool player_down_jump(Move *mv, Transform *tr)
{
    if (mv->foo->forbidFallDown == false)
    {
        auto view = World::registry->view<FootHold>();
        for (auto &e : view)
        {
            auto fh = &view.get<FootHold>(e);
            if (fh->get_y(tr->position.x).has_value() &&
                fh->get_y(tr->position.x).value() < tr->position.y + 600 &&
                fh->get_y(tr->position.x).value() > tr->position.y + 10 &&
                fh->x2 > fh->x1 &&
                fh != mv->foo)
            {
                return true;
            }
        }
    }
    return false;
}

int player_attack(Move *mv, Character *cha, Transform *tr, int state, entt::entity ent)
{
    if (Input::state[SDL_SCANCODE_LCTRL])
    {
        if (state != Character::State::JUMP)
        {
            mv->hspeed = 0;
        }
        // add afterimg
        World::registry->emplace_or_replace<AfterImage>(ent);

        state = Character::State::ATTACK;
        player_alert_cooldown = 4000;
        cha->animated = false;
    }
    return state;
}

bool player_animating(Move *mv, Character *cha, Transform *tr, entt::entity ent, float delta_time)
{
    if (mv->lr != nullptr && mv->hspeed == 0 && (cha->action == Character::ACTION::LADDER || cha->action == Character::ACTION::ROPE))
    {
        // 绳子或梯子上
        cha->state = Character::State::CLIMB;
        return true;
    }
    else if (mv->foo == nullptr)
    {
        if (player_fall(mv, tr, delta_time))
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

bool player_climb(Move *mv, Transform *tr, int state)
{
    if (player_ladderrope_cooldown <= 0)
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
    }
    return false;
}

int player_climbing(Character *cha, Move *mv, Transform *tr, int state, entt::entity ent, float delta_time)
{
    if (state == Character::State::JUMP)
    {
        cha->animate = true;
        return state;
    }
    state = Character::State::CLIMB;
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
            player_ladderrope_cooldown = 80;
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
        player_ladderrope_cooldown = 80;
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

void player_action(Character *cha, int state, int new_state, Move *mv)
{
    if (state != new_state)
    {
        int action = new_state;
        switch (action)
        {
        case Character::State::STAND:
        {
            auto weaponinfo = World::registry->try_get<WeaponInfo>(Player::ent);
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
        case Character::State::WALK:
        {
            auto weaponinfo = World::registry->try_get<WeaponInfo>(Player::ent);
            if (weaponinfo != nullptr && weaponinfo->walk1 == false)
            {
                action = Character::ACTION::WALK2;
            }
            else
            {
                action = Character::ACTION::WALK1;
            }
        }
        break;
        case Character::State::JUMP:
        {
            action = Character::ACTION::JUMP;
        }
        break;
        case Character::State::ATTACK:
        {
            if (state == Character::State::PRONE)
            {
                action = Character::ACTION::PRONESTAB;
            }
            else
            {
                action = player_attack_action(World::registry->try_get<WeaponInfo>(Player::ent));
            }
            if (auto aft = World::registry->try_get<AfterImage>(Player::ent))
            {
                auto weaponinfo = World::registry->try_get<WeaponInfo>(Player::ent);
                auto afterImage_index = AfterImage::afterImage_index(weaponinfo->reqLevel);
                aft->aspr = AnimatedSprite(AfterImage::afterimages[weaponinfo->afterImage][afterImage_index][action].asprw);
                aft->info = AfterImage::afterimages[weaponinfo->afterImage][afterImage_index][action];
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
        {
            action = Character::ACTION::PRONE;
        }
        break;
        case Character::State::ALERT:
        {
            action = Character::ACTION::ALERT;
        }
        break;
        case Character::State::SKILL:
        {
            if (cha->action_str == u"")
            {
                // 从攻击动作随机选择一个
                action = player_attack_action(World::registry->try_get<WeaponInfo>(Player::ent));
            }
            else if (Character::type_map.contains(cha->action_str))
            {
                action = Character::type_map.at(cha->action_str);
            }
            else
            {
                // 特殊动作
                action = -1;
            }
        }
        break;
        case Character::State::SIT:
        {
            action = Character::ACTION::SIT;
        }
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
    if (player_alert_cooldown > 0)
    {
        player_alert_cooldown -= delta_time;
    }
    if (World::registry->try_get<Character>(Player::ent)->invincible_cooldown > 0)
    {
        World::registry->try_get<Character>(Player::ent)->invincible_cooldown -= delta_time;
    }
    if (player_ladderrope_cooldown > 0)
    {
        player_ladderrope_cooldown -= delta_time;
    }
    player_face_cooldown -= delta_time;
}

void player_alert(Character *cha)
{
    player_alert_cooldown = 5000;
    player_face_cooldown = 5000;
    cha->face_type = u"hit";
    cha->face_index = u"0";
    cha->add_face(cha->face_str, cha->face_type, cha->face_index);
    return;
}

bool player_hit(Hit *hit, entt::entity ent)
{
    auto cha = World::registry->try_get<Character>(ent);
    if (hit->damage > 0 && cha->invincible_cooldown <= 0)
    {
        cha->invincible_cooldown = 2000;
        auto mv = World::registry->try_get<Move>(ent);
        auto tr = World::registry->try_get<Transform>(ent);
        auto cha = World::registry->try_get<Character>(ent);
        cha->hp -= hit->damage;
        hit->damage = 0;
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
                else if (cha_x == hit_x)
                {
                    auto tr = World::registry->try_get<Transform>(ent);
                    if (tr->flip)
                    {
                        mv->hspeed = 110;
                    }
                    else
                    {
                        mv->hspeed = -110;
                    }
                }
                else
                {
                    mv->hspeed = 110;
                }
                mv->foo = nullptr;
            }
            player_alert(cha);
            if (cha->state == Character::State::STAND || cha->state == Character::State::WALK ||
                cha->state == Character::State::ALERT || cha->state == Character::State::PRONE ||
                cha->state == Character::State::SIT)
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
                float y = World::registry->ctx().get<Border>().b.value();
                for (auto &e : World::registry->view<FloorFootHold>())
                {
                    auto fh = World::registry->try_get<FootHold>(e);
                    if (fh->get_y(tr->position.x).has_value() && fh->get_y(tr->position.x).value() > tr->position.y)
                    {
                        y = std::min(y, (float)fh->get_y(tr->position.x).value());
                    }
                }
                tr->position.y = y;
            }

            auto &tomb = World::registry->emplace_or_replace<Tomb>(ent);
            tomb.f.position = tr->position;
            tomb.f.position.y -= 200;
            tomb.l.position = tr->position;

            Sound::push(Sound(u"Game.img/Tombstone"), 180);
        }
        World::registry->remove<Install>(Player::ent);
        return true;
    }
    else if (hit->damage < 0)
    {
        cha->hp -= hit->damage;
        hit->damage = 0;
        return false;
    }
    else
    {
        return false;
    }
}

bool player_skill(Move *mv, Character *cha, Transform *tr, int state, entt::entity ent, std::u16string id)
{
    if (id == u"")
    {
        if (Input::state[SDL_SCANCODE_A])
        {
            id = u"2201005";
        }
        else if (Input::state[SDL_SCANCODE_S])
        {
            id = u"1311006";
        }
        else if (Input::state[SDL_SCANCODE_SPACE])
        {
            id = u"2201002";
        }
        else if (Input::state[SDL_SCANCODE_D])
        {
            id = u"2101004";
        }
        else if (Input::state[SDL_SCANCODE_F])
        {
            id = u"4201005";
        }
        else if (Input::state[SDL_SCANCODE_G])
        {
            id = u"2001004";
        }
        else if (Input::state[SDL_SCANCODE_C])
        {
            id = u"2301002";
        }
        else if (Input::state[SDL_SCANCODE_V])
        {
            id = u"4001344";
        }
        else if (Input::state[SDL_SCANCODE_B])
        {
            id = u"4211006";
        }
    }

    if (id != u"" && SkillWarp::cooldowns[id] <= 0)
    {
        SkillWarp::cooldowns[id] = 200;
        int skill_res = -1;
        if (PlayerSkill::Skills.contains(id))
        {
            skill_res = PlayerSkill::Skills[id]();
            if (skill_res == -1)
            {
                return false;
            }
        }
        else
        {
            // 通用攻击技能
            if (state == Character::State::CLIMB)
            {
                return false;
            }
            else if (state != Character::State::JUMP)
            {
                mv->hspeed = 0;
            }
            player_alert_cooldown = 4000;
        }
        Skill *ski = World::registry->try_get<Skill>(ent);
        if (ski == nullptr)
        {
            ski = &World::registry->emplace_or_replace<Skill>(ent, id);
        }
        // 技能效果
        if (skill_res & PlayerSkill::SkillResult::EFF)
        {
            PlayerSkill::skill_effect(ski);
        }

        // 技能音效
        if (skill_res & PlayerSkill::SkillResult::SOU)
        {
            PlayerSkill::skill_sound(ski->ski);
        }

        // 技能攻击
        if (skill_res & PlayerSkill::SkillResult::ATK)
        {
            PlayerSkill::skill_attack(ski);
        }

        // 人物状态
        if (skill_res & PlayerSkill::SkillResult::ACT)
        {
            PlayerSkill::skill_action(ski);
        }
        // 人物状态
        if (skill_res & PlayerSkill::SkillResult::ALERT)
        {
            player_alert_cooldown = 4000;
        }
        return true;
    }
    return false;
}

bool player_skilling(Move *mv, Character *cha, Transform *tr, entt::entity ent, float delta_time)
{
    player_quick_move(mv, cha, tr, cha->state, ent);
    if (!player_animating(mv, cha, tr, ent, delta_time))
    {
        World::registry->remove<Skill>(ent);
        return false;
    }
    return true;
}

void player_portal(Move *mv, entt::entity ent)
{
    if (player_portal_cooldown <= 0)
    {
        auto tr = World::registry->try_get<Transform>(ent);
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
                    if (player_pos.x == std::clamp(player_pos.x, por_x - 40, por_x + 40) &&
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
                            auto eff = World::registry->try_get<Effect>(ent);
                            eff->effects.push_back({new Transform(tr->position.x, tr->position.y), AnimatedSprite(Effect::load(u"BasicEff.img/Summoned"))});
                            eff->effects.push_back({nullptr, AnimatedSprite(Effect::load(u"BasicEff.img/Summoned"))});

                            auto position = std::get<SDL_FPoint>(por->tn);
                            tr->position.x = position.x;
                            tr->position.y = position.y - 5;
                            auto cha = World::registry->try_get<Character>(Player::ent);
                            cha->state = Character::State::JUMP;
                        }
                        mv->hspeed = 0;
                        mv->vspeed = 0;
                        player_portal_cooldown = 800;
                        break;
                    }
                }
            }
        }
    }
}

bool player_double_jump(Move *mv, Transform *tr, entt::entity ent)
{
    // 二段跳
    if (Input::state[SDL_SCANCODE_LALT] && SkillWarp::cooldowns[u"4111006"] <= 0)
    {
        mv->vspeed -= 420;
        if (tr->flip == 1)
        {
            // 朝右
            mv->hspeed += 480;
        }
        else
        {
            mv->hspeed -= 480;
        }
        // 添加effect
        auto eff = World::registry->try_get<Effect>(ent);
        eff->effects.push_back({new Transform(tr->position.x, tr->position.y), AnimatedSprite(Effect::load(u"BasicEff.img/Flying"))});

        // 技能音效
        auto ski = SkillWarp::load(u"4111006");
        auto souw = ski->sounds[u"Use"];

        Sound::push(souw);

        SkillWarp::cooldowns[u"4111006"] = 10000;
        return true;
    }
    return false;
}

uint8_t player_attack_action(WeaponInfo *wea)
{
    uint8_t action = 0;
    action = wea->attack_stances[wea->attack][std::rand() % wea->attack_stances[wea->attack].size()];
    // 判断是远程武器，且没有放技能
    if (WeaponInfo::if_long_range_weapon(wea->attack) && !World::registry->all_of<Skill>(Player::ent))
    {
        // 需要判断面前是否有怪物,否则切换到近战
        for (auto ent : World::registry->view<Damage, Mob>())
        {
            auto mob = World::registry->try_get<Mob>(ent);
            if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
            {
                auto m_tr = World::registry->try_get<Transform>(ent);
                auto p_tr = World::registry->try_get<Transform>(Player::ent);
                if (std::abs(p_tr->position.y - m_tr->position.y) <= 5)
                {
                    if ((p_tr->flip == 1 && p_tr->position.x <= m_tr->position.x && (m_tr->position.x - p_tr->position.x) <= 95) ||
                        (p_tr->flip == 0 && p_tr->position.x >= m_tr->position.x && (p_tr->position.x - m_tr->position.x) <= 95))
                    {
                        action = wea->degen_stances[wea->attack][std::rand() % wea->degen_stances[wea->attack].size()];
                        return action;
                    }
                }
            }
        }
        // 判断前面是否有触发器Reactor
        for (auto ent : World::registry->view<Reactor>())
        {
            auto r = World::registry->try_get<Reactor>(ent);
            // 判断是否有hit事件
            for (auto &e : r->a[r->index].event)
            {
                if (e.type == 0)
                {
                    // 说明可以进行hit
                    auto r_tr = World::registry->try_get<Transform>(ent);
                    auto p_tr = World::registry->try_get<Transform>(Player::ent);
                    if (std::abs(p_tr->position.y - r_tr->position.y) <= 30)
                    {
                        if ((p_tr->flip == 1 && p_tr->position.x <= r_tr->position.x && (r_tr->position.x - p_tr->position.x) <= 95) ||
                            (p_tr->flip == 0 && p_tr->position.x >= r_tr->position.x && (p_tr->position.x - r_tr->position.x) <= 95))
                        {
                            action = wea->degen_stances[wea->attack][std::rand() % wea->degen_stances[wea->attack].size()];
                            return action;
                        }
                    }
                }
            }
        }
    }
    return action;
}

bool player_pick_drop(Character *cha, Transform *tr)
{
    if (Input::state[SDL_SCANCODE_Z])
    {
        // 捡起物品
        for (auto ent : World::registry->view<Drop>())
        {
            auto dro = World::registry->try_get<Drop>(ent);
            if (dro->picker == entt::null && dro->land == true)
            {
                auto player_pos = tr->position;
                auto dro_tr = World::registry->try_get<Transform>(ent);
                auto dro_x = dro_tr->position.x;
                auto dro_y = dro_tr->position.y;
                if (player_pos.x == std::clamp(player_pos.x, dro_x - 20, dro_x + 40) &&
                    player_pos.y == std::clamp(player_pos.y, dro_y - 20, dro_y + 20))
                {
                    // 捡起物品
                    dro->picker = Player::ent;
                    dro->destory = Window::dt_now + 600;

                    auto mv = World::registry->try_get<Move>(ent);
                    mv->vspeed = -430;

                    // 播放声音
                    Sound::push(Sound(u"Game.img/PickUpItem"));
                    return true;
                }
            }
        }
    }
    return false;
}

void player_quick_move(Move *mv, Character *cha, Transform *tr, int state, entt::entity ent)
{
    if (Input::state[SDL_SCANCODE_SPACE])
    {
        player_skill(mv, cha, tr, state, ent, u"2201002");
    }
}

void player_face(Character *cha)
{
    if (Input::state[SDL_SCANCODE_F1])
    {
        cha->face_type = u"hit";
        cha->face_index = u"0";
        cha->face_time = 0;
        player_face_cooldown = 2000;
        cha->add_face(cha->face_str, cha->face_type, cha->face_index);
    }
    else if (Input::state[SDL_SCANCODE_F2])
    {
        cha->face_type = u"smile";
        cha->face_index = u"0";
        cha->face_time = 0;
        player_face_cooldown = 2000;
        cha->add_face(cha->face_str, cha->face_type, cha->face_index);
    }
    else if (Input::state[SDL_SCANCODE_F4])
    {
        cha->face_type = u"angry";
        cha->face_index = u"0";
        cha->face_time = 0;
        player_face_cooldown = 2000;
        cha->add_face(cha->face_str, cha->face_type, cha->face_index);
    }
    if (player_face_cooldown < 0 && player_face_cooldown > -2000 &&
        !(cha->face_type == u"blink" || cha->face_type == u"default"))
    {
        cha->face_type = u"default";
        cha->face_index = u"";
        player_face_cooldown = 0;
        cha->add_face(cha->face_str, cha->face_type, cha->face_index);
    }
    else if (player_face_cooldown <= -2000)
    {
        // blink
        cha->face_type = u"blink";
        cha->face_index = u"0";
        cha->face_time = 0;
        player_face_cooldown = 0;
        cha->add_face(cha->face_str, cha->face_type, cha->face_index);
    }
}

bool player_sit(Move *mv, int state)
{
    static bool keyborard = true;
    bool r = false;
    if (Input::state[SDL_SCANCODE_X])
    {
        if (keyborard)
        {
            if (player_alert_cooldown <= 0 && state != Character::State::SIT && mv->foo)
            {
                r = true;
                mv->hspeed = 0;
                World::registry->emplace_or_replace<Install>(Player::ent, u"03010048");
            }
            keyborard = false;
        }
        else if (state == Character::State::SIT)
        {
            r = true;
            mv->hspeed = 0;
        }
    }
    else
    {
        if (state == Character::State::SIT)
        {
            r = true;
            mv->hspeed = 0;
        }
        keyborard = true;
    }
    if (state == Character::State::SIT && keyborard == true)
    {
        if (Input::state[SDL_SCANCODE_LALT] ||
            Input::state[SDL_SCANCODE_LEFT] ||
            Input::state[SDL_SCANCODE_RIGHT])
        {
            r = false;
        }
    }
    if (r == false)
    {
        World::registry->remove<Install>(Player::ent);
    }
    return r;
}
