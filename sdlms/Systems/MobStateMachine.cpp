#include "MobStateMachine.h"
#include "Move.h"
#include "Collision.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "Entities/Entities.h"
#include "Systems/Attack.h"
#include "UI/GainTip.h"

void mob_statemachine_run()
{
    auto view = World::registry->view<Mob>();
    for (auto ent : view)
    {
        mob_collision_attack(ent);
        auto mob = World::registry->try_get<Mob>(ent);
        if (mob->state == Mob::State::REMOVE)
        {
            mob_revive(ent, Window::delta_time);
            continue;
        }
        bool res = true;
        for (auto it = mob->call_backs.begin(); it != mob->call_backs.end();)
        {

            const auto &key = it->first;     // 获取键
            auto &[func, data] = it->second; // 获取值，即一个函数对象
            auto pair = (func)(ent, data);
            auto r = pair.first;
            if (r)
            {
                it = mob->call_backs.erase(it);
            }
            else
            {
                ++it;
            }
            if (pair.second == false)
            {
                res = false;
            }
        }
        if (res)
        {
            mob_statemachine(ent, (float)Window::delta_time / 1000);
        }
    }
}

void mob_statemachine(entt::entity ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto mob = World::registry->try_get<Mob>(ent);

    auto state = mob->state;
    switch (state)
    {
    case Mob::State::STAND:
    {
        if (mob_attack(mob, tr))
        {
            mob->state = Mob::State::ATTACK;
        }
    }
    break;
    case Mob::State::MOVE:
    {
        mob_flip(mv, tr);
        if (mob_attack(mob, tr))
        {
            mob->state = Mob::State::ATTACK;
            break;
        }
        if (mob_jump(mob, mv))
        {
            mob->state = Mob::State::JUMP;
        }
        else
        {
            mob->state = mob_move(mob, mv, tr, state, delta_time);
        }
    }
    break;
    case Mob::State::FLY:
    {
        mob_flip(mv, tr);
        mob->state = mob_fly(mob, mv, tr, state, delta_time);
    }
    break;
    case Mob::State::STAY:
    {
        return;
    }
    break;
    case Mob::State::JUMP:
    case Mob::State::HIT:
    {
        mob_fall(ent, delta_time);
    }
    break;
    case Mob::State::DIE:
    case Mob::State::REMOVE:
    {
    }
    break;
    }
    mob->state = mob_active(mob, mv, tr, mob->state, delta_time);
    mob_action(mob, mv, tr, state, mob->state);
}

void mob_flip(Move *mv, Transform *tr)
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

int mob_move(Mob *mob, Move *mv, Transform *tr, int state, float delta_time)
{
    move_move(mv, tr, 0, delta_time, false);
    return state;
}

int mob_fly(Mob *mob, Move *mv, Transform *tr, int state, float delta_time)
{
    move_fall(mv, tr, delta_time, tr->z, false, false);
    return state;
}

void mob_action(Mob *mob, Move *mv, Transform *tr, int state, int new_state)
{
    if (state != new_state)
    {
        int action = new_state;
        switch (action)
        {
        case Mob::State::STAND:
        {
            mob->tick = std::rand() % 100 + 2110 + Window::dt_now;
            mob->index = u"stand";
            mv->hspeed = 0;
        }
        break;
        case Mob::State::MOVE:
        {
            mob->tick = std::rand() % 100 + 2390 + Window::dt_now;
            mob->index = u"move";
            mob_set_hspeed(mob, mv, tr);
        }
        break;
        case Mob::State::JUMP:
        {
            if (mob->a.contains(u"jump"))
            {
                mob->index = u"jump";
            }
        }
        break;
        case Mob::State::HIT:
        {
            mob->index = u"hit1";
        }
        break;
        case Mob::State::ATTACK:
        {
            mob->index = u"attack1";
        }
        break;
        }
        mob->a[mob->index].anim_index = 0;
        mob->a[mob->index].anim_time = 0;
    }
}

optional<int> mob_hit(Attack *atk, entt::entity ent, std::optional<SDL_FPoint> hit_point)
{
    optional<int> full_damage = 0;

    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto mob = World::registry->try_get<Mob>(ent);

    if (atk->damage > 0)
    {
        mob->hit = Player::ent;
        if (hit_point.has_value())
        {
            Effect::push(World::registry->try_get<Effect>(ent), atk->hit, hit_point, tr->flip, true);
        }
        else
        {
            Effect::push(World::registry->try_get<Effect>(ent), atk->hit, hit_point, tr->flip, false);
        }
        for (int i = 0; i < atk->attackCount; i++)
        {
            auto r = generate_random(atk->min_damage, atk->max_damage);
            int damage = atk->damage * r;
            auto type = damage > atk->damage ? Damage::Type::Cri : Damage::Type::Red;
            // 一次攻击造成的伤害值
            full_damage.value() += damage;

            mob->hp -= damage;
            // 怪物被攻击音效
            if (atk->souw)
            {
                Sound::push(atk->souw, i * 140);
            }
            else if (mob->sounds.contains(u"Damage"))
            {
                Sound::push(mob->sounds[u"Damage"], i * 140);
            }
            Damage::push(World::registry->try_get<Damage>(ent), damage, type,
                         SDL_FPoint{tr->position.x,
                                    tr->position.y + (float)dynamic_cast<wz::Property<wz::WzVec2D> *>(mob->a[u"hit1"].asprw->sprites[0]->n->get_child(u"head"))->get().y},
                         Window::dt_now + i * 60);
        }
        if (mob->state != Mob::State::DIE)
        {
            if (mob->hp <= 0)
            {
                // 怪物死亡音效
                if (mob->sounds.contains(u"Die"))
                {
                    Sound::push(mob->sounds[u"Die"], 200);
                }
                mob->state = Mob::State::DIE;
                mob->index = u"die1";

                // 爆金币
                mob_drop(mob, tr);
                // 获取经验提示
                GainTip::push(u"e", Window::dt_now);

                mob->revive = mob->revive_time + Window::dt_now;
            }
            else if (mob_hit_move(atk->src_point, ent))
            {
                mob->tick = Window::dt_now;
                mob->state = Mob::State::HIT;
                mob->index = u"hit1";
            }
        }
    }
    return full_damage;
}

bool mob_hit_move(std::optional<SDL_FPoint> &point, entt::entity ent)
{
    if (point.has_value())
    {
        auto mv = World::registry->try_get<Move>(ent);
        auto tr = World::registry->try_get<Transform>(ent);
        auto mob = World::registry->try_get<Mob>(ent);
        auto hit_x = point.value().x;
        auto mob_x = tr->position.x;
        tr->flip = mob_x < hit_x ? 1 : 0;
        mv->hspeed = mob_x < hit_x ? -125 : 125;
        if (mob->a.contains(u"fly"))
        {
            mv->vspeed = 0;
            mob_fly(mob, mv, tr, mob->state, 0.13);
            mv->hspeed = 0;
        }
        else if (mv->foo)
        {
            mob_move(mob, mv, tr, mob->state, 0.13);
            mv->hspeed = 0;
        }
        return true;
    }
    return false;
}

void mob_drop(Mob *mob, Transform *tr)
{
    std::vector<std::pair<std::u16string, unsigned int>> drops = {{u"09000000", (unsigned int)std::rand() % 200}};
    for (auto it : mob->drops[mob->id])
    {
        drops.push_back({it, 1});
    }
    load_drops(drops, tr->position.x, tr->position.y, tr->z / LAYER_Z);
    Sound::push(Sound(u"Game.img/DropItem", 300));
    World::zindex = true;
}

bool mob_fall(entt::entity ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto mob = World::registry->try_get<Mob>(ent);

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
            mob->tick = Window::dt_now;
            return false;
        }
    }
}

int mob_active(Mob *mob, Move *mv, Transform *tr, int state, float delta_time)
{
    if (!(state == Mob::State::ATTACK || state == Mob::State::HIT || state == Mob::State::DIE ||
          state == Mob::State::REMOVE || mv->foo == nullptr))
    {
        if (mob->tick < Window::dt_now)
        {
            if (state == Mob::State::FLY)
            {
                mob->tick = std::rand() % 100 + 200 + Window::dt_now;
                mob_set_hspeed(mob, mv, tr);
                mob_set_vspeed(mob, mv, tr);
                if (World::registry->valid(mob->hit))
                {
                    mv->rx0 = std::nullopt;
                    mv->rx1 = std::nullopt;
                    mv->ry0 = std::nullopt;
                    mv->ry1 = std::nullopt;
                }
            }
            else if (mv->foo)
            {
                if (World::registry->valid(mob->hit))
                {
                    mob->tick = std::rand() % 100 + 200 + Window::dt_now;
                    mob_set_hspeed(mob, mv, tr);
                    state = Mob::State::MOVE;
                }
                else
                {
                    // 只有在地面才可以切换状态
                    //  怪物状态切换 STAND MOVE
                    mob->tick = std::rand() % 100 + 240 + Window::dt_now;
                    int random = std::rand() % 2;
                    switch (random)
                    {
                    case 0:
                        state = Mob::State::STAND;
                        mv->hspeed = 0;
                        break;
                    case 1:
                        state = Mob::State::MOVE;
                        mob_set_hspeed(mob, mv, tr);
                        break;
                    }
                }
            }
        }
    }
    return state;
}

bool mob_revive(entt::entity ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto mob = World::registry->try_get<Mob>(ent);

    if (mob->revive <= Window::dt_now)
    {
        if (mob->a.contains(u"move"))
        {
            mob->state = Mob::State::MOVE;
        }
        else if (mob->a.contains(u"stand"))
        {
            mob->state = Mob::State::STAY;
        }
        else if (mob->a.contains(u"fly"))
        {
            mob->state = Mob::State::FLY;
        }
        return true;
    }
    else if (mob->revive <= Window::dt_now + mob->revive_alpha_time + 100)
    {
        World::registry->try_get<Effect>(ent)->effects.clear();
        mob->call_backs.clear();

        mob->hp = 100;
        mob->hit = entt::null;
        if (mob->a.contains(u"fly"))
        {
            mv->ry0 = mob->init_y - 40;
            mv->ry1 = mob->init_y + 40;
        }
        mv->foo = mob->init_fh;
        tr->position.x = mob->init_x;
        tr->position.y = mob->init_y;
    }
    return false;
}

bool mob_attack(Mob *mob, Transform *tr)
{
    if (World::registry->valid(mob->hit) && mob->a.contains(u"attack1"))
    {
        // 怪物攻击
        auto h_tr = World::registry->try_get<Transform>(mob->hit);
        if (std::abs(h_tr->position.x - tr->position.x) < 200 &&
            std::abs(h_tr->position.y - tr->position.y) < 15)
        {
            return true;
        }
    }
    return false;
}

bool mob_jump(Mob *mob, Move *mv)
{
    if (mob->a.contains(u"jump"))
    {
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
            int random = std::rand() % 35;
            if (random == 0)
            {
                mv->foo = nullptr;
                mv->vspeed = -420;
                return true;
            }
        }
    }
    return false;
}

bool mob_collision_attack(entt::entity ent)
{
    auto player_character = World::registry->try_get<Character>(Player::ent);
    if (player_character->invincible_cooldown <= Window::dt_now)
    {
        auto mob = World::registry->try_get<Mob>(ent);
        if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
        {
            auto mob_transform = World::registry->try_get<Transform>(ent);
            auto player_transform = World::registry->try_get<Transform>(Player::ent);
            if (collision(mob->rect(), mob_transform, player_character->r, player_transform))
            {
                Attack atk = mob->atk;
                atk.damage = 800;
                atk.src_point = mob_transform->position;
                atk.hit = nullptr;
                attack_player(&atk, ent, std::nullopt);
                return true;
            }
        }
    }
    return false;
}

void mob_set_hspeed(Mob *mob, Move *mv, Transform *tr)
{
    auto ent = mob->hit;
    if (World::registry->valid(ent))
    {
        auto m_tr = tr;
        auto h_tr = World::registry->try_get<Transform>(ent);
        if (h_tr != nullptr)
        {
            mv->hspeed = (h_tr->position.x > m_tr->position.x)
                             ? mv->hspeed_max.value()
                             : mv->hspeed_min.value();
        }
    }
    else
    {
        int random = std::rand() % 2;
        switch (random)
        {
        case 0:
        {
            mv->hspeed = mv->hspeed_min.value();
            break;
        }
        case 1:
        {
            mv->hspeed = mv->hspeed_max.value();
            break;
        }
        }
    }
    return;
}

void mob_set_vspeed(Mob *mob, Move *mv, Transform *tr)
{
    auto ent = mob->hit;
    if (World::registry->valid(ent))
    {
        auto m_tr = tr;
        auto h_tr = World::registry->try_get<Transform>(ent);
        if (h_tr != nullptr)
        {
            mv->vspeed = (h_tr->position.y > m_tr->position.y)
                             ? mv->hspeed_max.value()
                             : mv->hspeed_min.value();
        }
    }
    else
    {
        int random = std::rand() % 2;
        switch (random)
        {
        case 0:
        {
            mv->vspeed = mv->hspeed_min.value();
            break;
        }
        case 1:
        {
            mv->vspeed = mv->hspeed_max.value();
            break;
        }
        }
    }
    return;
}
