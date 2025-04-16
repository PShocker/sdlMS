#include "PlayerStateMachine.h"
#include "Move.h"
#include "Pick.h"
#include "entt/entt.hpp"
#include "Commons/Commons.h"
#include "PlayerSkill/PlayerSkill.h"
#include "Core/Core.h"
#include <SDL3/SDL.h>
#include <optional>
#include "Systems/Attack.h"
#include "UI/UI.h"

void player_statemachine_run()
{
    if (auto ent = Player::ent; World::registry->valid(ent))
    {
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
        player_pick_drop(ent);
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
        player_pick_drop(ent);
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
        cha->invincible_cooldown = Window::dt_now + 2000;
        player_face(cha);
        return;
    }
    break;
    case Character::State::SIT:
    {
        player_pick_drop(ent);
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
    if (Input::state[Keyboard::action[Keyboard::Action::RIGHT]])
    {
        tr->flip = 1;
    }
    else if (Input::state[Keyboard::action[Keyboard::Action::LEFT]])
    {
        tr->flip = 0;
    }
}

int player_walk(Move *mv, Transform *tr, float delta_time)
{
    auto state = Character::State::WALK;

    if (Input::state[Keyboard::action[Keyboard::Action::RIGHT]])
    {
        mv->hforce = 1400;
    }
    else if (Input::state[Keyboard::action[Keyboard::Action::LEFT]])
    {
        mv->hforce = -1400;
    }
    else
    {
        if (player_alert_cooldown > Window::dt_now)
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
    if (Input::state[Keyboard::action[Keyboard::Action::RIGHT]])
    {
        mv->hspeed += 0.3f;
    }
    else if (Input::state[Keyboard::action[Keyboard::Action::LEFT]])
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
    auto r = move_fall(mv, tr, delta_time, CHARACTER_Z, player_foothold_cooldown <= Window::dt_now);
    if (r == false)
    {
        auto character = World::registry->try_get<Character>(Player::ent);
        if (character->invincible_cooldown <= Window::dt_now)
        {
            auto distance = tr->position.y - player_fall_y;
            if (distance >= 600)
            {
                Attack atk;
                atk.damage = distance / 25;
                atk.hit = nullptr;
                atk.src_point = tr->position;
                if (attack_player(&atk, Player::ent, std::nullopt))
                {
                    r = true;
                }
            }
        }
    }
    return r;
}

bool player_jump(Move *mv, Character *cha, Transform *tr, int state)
{
    if (Input::state[Keyboard::action[Keyboard::Action::JUMP]])
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
                    Skill::cooldowns[4111006] = Window::dt_now + 500;
                    player_foothold_cooldown = Window::dt_now + 120;
                    Sound::push(Sound(u"Game.img/Jump"));
                    return true;
                }
            }
            else
            {
                mv->vspeed = mv->vspeed_min.value() * 0.111;
                mv->page = mv->foo->page;
                mv->zmass = mv->foo->zmass;
                mv->foo = nullptr;
                mv->lr = nullptr;
                Skill::cooldowns[4111006] = Window::dt_now + 200;
                Sound::push(Sound(u"Game.img/Jump"));
                return true;
            }
        }
        else if (mv->lr)
        {
            if (!Input::state[Keyboard::action[Keyboard::Action::UP]] && (Input::state[Keyboard::action[Keyboard::Action::RIGHT]] || Input::state[Keyboard::action[Keyboard::Action::LEFT]]))
            {
                cha->animate = true;

                mv->vspeed = -310;

                if (Input::state[Keyboard::action[Keyboard::Action::RIGHT]])
                {
                    mv->hspeed = 140;
                }
                else if (Input::state[Keyboard::action[Keyboard::Action::LEFT]])
                {
                    mv->hspeed = -140;
                }
                Skill::cooldowns[4111006] = Window::dt_now + 300;
                player_ladderrope_cooldown = Window::dt_now + 80;
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
    if (Input::state[Keyboard::action[Keyboard::Action::ATTACK]])
    {
        if (state != Character::State::JUMP)
        {
            mv->hspeed = 0;
        }
        // add afterimg
        World::registry->emplace_or_replace<AfterImage>(ent);

        state = Character::State::ATTACK;
        player_alert_cooldown = Window::dt_now + 4000;
        cha->animated = false;
    }
    return state;
}

bool player_animating(Move *mv, Character *cha, Transform *tr, entt::entity ent, float delta_time)
{
    if (cha->action == Character::Action::LADDER || cha->action == Character::Action::ROPE)
    {
        // 绳子或梯子上
        if (player_climb_cooldown > Window::dt_now)
        {
            return true;
        }
        else
        {
            cha->state = Character::State::CLIMB;
            return false;
        }
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
    if (player_ladderrope_cooldown <= Window::dt_now)
    {
        if (Input::state[Keyboard::action[Keyboard::Action::UP]] || Input::state[Keyboard::action[Keyboard::Action::DOWN]])
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
                        if (Input::state[Keyboard::action[Keyboard::Action::DOWN]])
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
                    if (!(mv->foo == nullptr && Input::state[Keyboard::action[Keyboard::Action::DOWN]]))
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
    if (Input::state[Keyboard::action[Keyboard::Action::DOWN]])
    {
        mv->vspeed = 100;
        cha->animate = true;
    }
    else if (Input::state[Keyboard::action[Keyboard::Action::UP]])
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
            player_ladderrope_cooldown = Window::dt_now + 80;
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
        player_ladderrope_cooldown = Window::dt_now + 80;
        cha->animate = true;
    }
    return state;
}

bool player_prone(Move *mv, Transform *tr, int state)
{
    if (Input::state[Keyboard::action[Keyboard::Action::DOWN]])
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
    if (Input::state[Keyboard::action[Keyboard::Action::DOWN]])
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
            auto weaponWrap = World::registry->try_get<WeaponWrap>(Player::ent);
            if (weaponWrap != nullptr && weaponWrap->stand1 == false)
            {
                action = Character::Action::STAND2;
            }
            else
            {
                action = Character::Action::STAND1;
            }
            cha->r = SDL_FRect{-20, -50, 30, 45};
        }
        break;
        case Character::State::WALK:
        {
            auto weaponWrap = World::registry->try_get<WeaponWrap>(Player::ent);
            if (weaponWrap != nullptr && weaponWrap->walk1 == false)
            {
                action = Character::Action::WALK2;
            }
            else
            {
                action = Character::Action::WALK1;
            }
            cha->r = SDL_FRect{-20, -50, 30, 45};
        }
        break;
        case Character::State::JUMP:
        {
            action = Character::Action::JUMP;
            cha->r = SDL_FRect{-20, -50, 30, 45};
        }
        break;
        case Character::State::ATTACK:
        {
            if (state == Character::State::PRONE)
            {
                action = Character::Action::PRONESTAB;
            }
            else
            {
                action = player_attack_action(World::registry->try_get<WeaponWrap>(Player::ent));
                cha->r = SDL_FRect{-20, -50, 30, 45};
            }
            if (auto aft = World::registry->try_get<AfterImage>(Player::ent))
            {
                auto weaponWrap = World::registry->try_get<WeaponWrap>(Player::ent);
                aft->aspr = AnimatedSprite(AfterImage::afterimages[weaponWrap->afterImage][weaponWrap->afterImage_index][action].asprw);
                aft->wrap = AfterImage::afterimages[weaponWrap->afterImage][weaponWrap->afterImage_index][action];
            }
        }
        break;
        case Character::State::CLIMB:
        {
            switch (mv->lr->l)
            {
            case 1:
                action = Character::Action::LADDER;
                break;
            default:
                action = Character::Action::ROPE;
                break;
            }
            cha->r = SDL_FRect{-20, -50, 30, 45};
        }
        break;
        case Character::State::PRONE:
        {
            action = Character::Action::PRONE;
            cha->r = SDL_FRect{-45, -30, 60, 25};
        }
        break;
        case Character::State::ALERT:
        {
            action = Character::Action::ALERT;
            cha->r = SDL_FRect{-20, -50, 30, 45};
        }
        break;
        case Character::State::SKILL:
        {
            if (cha->action_str == u"")
            {
                // 从攻击动作随机选择一个
                action = player_attack_action(World::registry->try_get<WeaponWrap>(Player::ent));
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
            cha->r = SDL_FRect{-20, -50, 30, 45};
        }
        break;
        case Character::State::SIT:
        {
            action = Character::Action::SIT;
            cha->r = SDL_FRect{-20, -50, 30, 45};
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

void player_alert(Character *cha)
{
    player_alert_cooldown = Window::dt_now + 4000;
    player_face_cooldown = Window::dt_now + 4000;
    cha->face_type = u"hit";
    cha->face_index = u"0";
    cha->add_face(cha->face_str, cha->face_type, cha->face_index);
    return;
}

int player_hit(Attack *atk)
{
    auto ent = Player::ent;

    int full_damage = 0;

    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);

    if (atk->damage > 0 && cha->invincible_cooldown <= Window::dt_now)
    {
        cha->invincible_cooldown = Window::dt_now + 2000;
        World::registry->remove<Install>(ent);

        Effect::push(World::registry->try_get<Effect>(ent), atk->hit, std::nullopt, tr->flip);
        for (int i = 0; i < atk->attackCount; i++)
        {
            auto r = generate_random(atk->min_damage, atk->max_damage);
            auto damage = atk->damage * r;
            full_damage += damage;

            Damage::push(World::registry->try_get<Damage>(ent), damage, Damage::Type::Violet, tr->position + SDL_FPoint{-15, -60});

            Player::hp -= damage;
            if (Player::hp > 0)
            {
                if (atk->souw)
                {
                    Sound::push(atk->souw);
                }
                if (mv->foo && cha->action != Character::Action::PRONESTAB)
                {
                    mv->vspeed = -320;
                    auto hit_x = atk->src_point.value().x;
                    auto cha_x = tr->position.x;
                    if (cha_x < hit_x)
                    {
                        mv->hspeed = -110;
                    }
                    else if (cha_x == hit_x)
                    {
                        mv->hspeed = tr->flip == 1 ? 110 : -110;
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
                    if (cha->state == Character::State::PRONE)
                    {
                        cha->r = SDL_FRect{-20, -50, 30, 45};
                    }
                    cha->state = Character::State::JUMP;
                    cha->action_index = 0;
                    cha->action_time = 0;
                    cha->action_frame = 0;
                    cha->action = Character::Action::JUMP;
                    cha->action_str = u"jump";
                }
            }
            else if (cha->state != Character::State::DIE)
            {
                World::registry->remove<Skill>(ent);
                World::registry->remove<AfterImage>(ent);

                cha->state = Character::State::DIE;
                cha->action_index = 0;
                cha->action_time = 0;
                cha->action_frame = 0;
                cha->action = Character::Action::DEAD;
                cha->action_str = u"dead";
                Player::hp = 0;

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

                Sound::push(Sound(u"Game.img/Tombstone"));
                // 弹出复活对话框
                UINotice::show();
            }
        }
    }
    else if (atk->damage < 0)
    {
        Damage::push(World::registry->try_get<Damage>(ent), -atk->damage, Damage::Type::Blue, tr->position + SDL_FPoint{-15, -60});
        Player::hp -= atk->damage;
        Player::hp = std::min(Player::hp, Player::max_hp);
    }
    return full_damage;
}

bool player_skill(Move *mv, Character *cha, Transform *tr, int state, entt::entity ent, int id)
{
    if (id == 0)
    {
        // 遍历映射表，检查是否有按键被按下
        for (const auto &[key, value] : Keyboard::skill)
        {
            if (Input::state[key])
            {
                id = value;
                break; // 找到一个按键后即可退出
            }
        }
    }
    if (id != 0)
    {
        if (Skill::cooldowns.contains(id) && Skill::cooldowns[id] > Window::dt_now)
        {
            return false;
        }
        int skill_res = -1;
        if (PlayerSkill::Skills.contains(id))
        {
            skill_res = PlayerSkill::Skills.at(id)(Player::ent);
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
            player_alert_cooldown = Window::dt_now + 4000;
        }
        Skill *ski = World::registry->try_get<Skill>(ent);
        if (ski == nullptr)
        {
            ski = &World::registry->emplace_or_replace<Skill>(ent, id);
        }
        // 技能效果
        if (skill_res & PlayerSkill::SkillResult::EFF)
        {
            PlayerSkill::skill_effect(ski, Player::ent);
        }

        // 技能音效
        if (skill_res & PlayerSkill::SkillResult::SOU)
        {
            PlayerSkill::skill_sound(ski);
        }

        // 技能攻击
        if (skill_res & PlayerSkill::SkillResult::ATK)
        {
            PlayerSkill::skill_attack(ski);
        }

        // 人物状态
        if (skill_res & PlayerSkill::SkillResult::ACT)
        {
            PlayerSkill::skill_action(ski, Player::ent);
        }
        // 人物状态
        if (skill_res & PlayerSkill::SkillResult::ALERT)
        {
            player_alert_cooldown = Window::dt_now + 4000;
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
    if (player_portal_cooldown <= Window::dt_now)
    {
        auto tr = World::registry->try_get<Transform>(ent);
        auto view = World::registry->view<Portal>();
        for (auto &e : view)
        {
            auto por = &view.get<Portal>(e);
            if (por->tm != 999999999)
            {
                if (por->pt == 3 || Input::state[Keyboard::action[Keyboard::Action::UP]])
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
                            eff->effects.emplace(0, Effect::Wrap{Transform(tr->position.x, tr->position.y), AnimatedSprite(Effect::load(u"BasicEff.img/Summoned"))});
                            eff->effects.emplace(0, Effect::Wrap{std::nullopt, AnimatedSprite(Effect::load(u"BasicEff.img/Summoned"))});

                            auto position = std::get<SDL_FPoint>(por->tn);
                            tr->position.x = position.x;
                            tr->position.y = position.y - 5;
                            auto cha = World::registry->try_get<Character>(Player::ent);
                            cha->state = Character::State::JUMP;
                            Sound::push(Sound(u"Game.img/Portal"));
                        }
                        mv->hspeed = 0;
                        mv->vspeed = 0;
                        player_portal_cooldown = Window::dt_now + 800;
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
    if (Input::state[Keyboard::action[Keyboard::Action::JUMP]])
    {
        if (Skill::cooldowns.contains(4111006) && Skill::cooldowns[4111006] > Window::dt_now)
        {
            return false;
        }

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
        eff->effects.emplace(4111006, Effect::Wrap{Transform(tr->position, 0, tr->flip), AnimatedSprite(Effect::load(u"BasicEff.img/Flying"))});

        // 技能音效
        auto ski = Skill::Wrap::load(4111006);
        auto souw = ski->sounds[u"Use"];

        Sound::push(souw);
        Skill::cooldowns[4111006] = Window::dt_now + 500;
        return true;
    }
    return false;
}

uint8_t player_attack_action(WeaponWrap *wea)
{
    uint8_t action = 0;
    action = wea->attack_stances[wea->attack][std::rand() % wea->attack_stances[wea->attack].size()];
    // 判断是远程武器，且没有放技能
    if (WeaponWrap::if_long_range_weapon(wea->attack) && !World::registry->all_of<Skill>(Player::ent))
    {
        // 需要判断面前是否有怪物,否则切换到近战
        for (auto ent : World::registry->view<Mob>())
        {
            auto mob = World::registry->try_get<Mob>(ent);
            if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
            {
                auto m_tr = World::registry->try_get<Transform>(ent);
                auto p_tr = World::registry->try_get<Transform>(Player::ent);
                if (std::abs(p_tr->position.y - m_tr->position.y) <= 10)
                {
                    if ((p_tr->flip == 1 && p_tr->position.x <= m_tr->position.x && (m_tr->position.x - p_tr->position.x) <= 75) ||
                        (p_tr->flip == 0 && p_tr->position.x >= m_tr->position.x && (p_tr->position.x - m_tr->position.x) <= 75))
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
                if (e.first == 0)
                {
                    // 说明可以进行hit
                    auto r_tr = World::registry->try_get<Transform>(ent);
                    auto p_tr = World::registry->try_get<Transform>(Player::ent);
                    if (std::abs(p_tr->position.y - r_tr->position.y) <= 30)
                    {
                        if ((p_tr->flip == 1 && p_tr->position.x <= r_tr->position.x && (r_tr->position.x - p_tr->position.x) <= 75) ||
                            (p_tr->flip == 0 && p_tr->position.x >= r_tr->position.x && (p_tr->position.x - r_tr->position.x) <= 75))
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

bool player_pick_drop(entt::entity ent)
{
    if (Input::state[Keyboard::action[Keyboard::Action::PICK]])
    {
        // 捡起物品
        pick_drop(ent);
    }
    return false;
}

void player_quick_move(Move *mv, Character *cha, Transform *tr, int state, entt::entity ent)
{
    // 获取瞬间移动的键位
    int key_code = 0;
    for (auto &[key, value] : Keyboard::skill)
    {
        if (value == 2201002)
        {
            key_code = key;
        }
    }
    if (Input::state[key_code])
    {
        player_skill(mv, cha, tr, state, ent, 2201002);
    }
}

void player_face(Character *cha)
{
    if (Input::state[SDL_SCANCODE_F1])
    {
        cha->face_type = u"hit";
        cha->face_index = u"0";
        cha->face_time = 0;
        player_face_cooldown = Window::dt_now + 2000;
        cha->add_face(cha->face_str, cha->face_type, cha->face_index);
    }
    else if (Input::state[SDL_SCANCODE_F2])
    {
        cha->face_type = u"smile";
        cha->face_index = u"0";
        cha->face_time = 0;
        player_face_cooldown = Window::dt_now + 2000;
        cha->add_face(cha->face_str, cha->face_type, cha->face_index);
    }
    else if (Input::state[SDL_SCANCODE_F4])
    {
        cha->face_type = u"angry";
        cha->face_index = u"0";
        cha->face_time = 0;
        player_face_cooldown = Window::dt_now + 2000;
        cha->add_face(cha->face_str, cha->face_type, cha->face_index);
    }
    if (player_face_cooldown < Window::dt_now && player_face_cooldown > Window::dt_now - 2000 &&
        !(cha->face_type == u"blink" || cha->face_type == u"default"))
    {
        cha->face_type = u"default";
        cha->face_index = u"";
        player_face_cooldown = Window::dt_now;
        cha->add_face(cha->face_str, cha->face_type, cha->face_index);
    }
    else if (player_face_cooldown <= Window::dt_now - 2000)
    {
        // blink
        cha->face_type = u"blink";
        cha->face_index = u"0";
        cha->face_time = 0;
        player_face_cooldown = Window::dt_now;
        cha->add_face(cha->face_str, cha->face_type, cha->face_index);
    }
}

bool player_sit(Move *mv, int state)
{
    static bool keyborard = true;
    bool r = false;
    if (Input::state[Keyboard::action[Keyboard::Action::SIT]])
    {
        if (keyborard)
        {
            if (player_alert_cooldown <= Window::dt_now && state != Character::State::SIT && mv->foo)
            {
                r = true;
                mv->hspeed = 0;
                World::registry->emplace_or_replace<Install>(Player::ent, u"03010000");
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
        if (Input::state[Keyboard::action[Keyboard::Action::JUMP]] ||
            Input::state[Keyboard::action[Keyboard::Action::LEFT]] ||
            Input::state[Keyboard::action[Keyboard::Action::RIGHT]])
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