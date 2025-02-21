#include "Ball.h"

#include "entt/entt.hpp"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

const std::unordered_set<std::u16string> track_skill =
    {u"4111005"};

entt::entity load_ball(AnimatedSpriteWarp *asprw, Transform *tran, entt::entity owner,
                       AnimatedSpriteWarp *hit, std::optional<int> rotate,
                       Skill *ski)
{
    auto ent = World::registry->create();
    auto &tr = World::registry->emplace<Transform>(ent);

    tr.position.y = tran->position.y - 30;
    tr.flip = tran->flip;
    tr.z = tran->z - 1;

    auto &ball = World::registry->emplace<Ball>(ent);
    ball.owner = owner;
    ball.hit = hit;
    ball.rotate = rotate;

    World::registry->emplace<Animated>(ent);
    World::registry->emplace<AnimatedSprite>(ent, asprw);

    if (ski != nullptr)
    {
        auto &s = World::registry->emplace<Skill>(ent);
        s.atkw = ski->atkw;
        if (track_skill.contains(ski->skiw->id))
        {
            // 这类技能不追踪怪物,水平直线移动,例如飞侠的多重飞镖
            ball.track = false;
            ball.track_hit = std::unordered_set<entt::entity>();
        }
    }

    auto &mv = World::registry->emplace<Move>(ent);
    mv.vspeed = 0;

    auto flip = tran->flip;
    if (flip)
    {
        mv.hspeed = 550;
        tr.position.x = tran->position.x;
    }
    else
    {
        mv.hspeed = -550;
        tr.position.x = tran->position.x;
    }
    World::zindex = true;
    return ent;
}

std::vector<entt::entity> load_ball(unsigned char num, std::optional<int> rotate, Skill *ski)
{
    std::vector<entt::entity> e;

    auto ent = Player::ent;
    auto tr = World::registry->try_get<Transform>(ent);
    auto weaponinfo = World::registry->try_get<WeaponInfo>(ent);

    std::u16string ball_path;
    if (weaponinfo->attack == WeaponInfo::Attack::BOW)
    {
        ball_path = u"Consume/0206.img/02060001/bullet";
        auto node = Wz::Item->get_root()->find_from_path(ball_path);
        auto asprw = AnimatedSpriteWarp::load(node);
        auto hit = Wz::Skill->get_root()->find_from_path(u"300.img/skill/3001005/CharLevel/10/hit/0");
        return load_ball(asprw, ent, num, AnimatedSpriteWarp::load(hit), rotate, ski);
    }
    else if (weaponinfo->attack == WeaponInfo::Attack::CLAW)
    {
        ball_path = u"Consume/0207.img/02070005/bullet";
        auto node = Wz::Item->get_root()->find_from_path(ball_path);
        auto asprw = AnimatedSpriteWarp::load(node);
        auto hit = Wz::Skill->get_root()->find_from_path(u"400.img/skill/4001344/CharLevel/20/hit/0");
        return load_ball(asprw, ent, num, AnimatedSpriteWarp::load(hit), rotate, ski);
    }
    return e;
}

std::vector<entt::entity> load_ball(AnimatedSpriteWarp *asprw, entt::entity owner,
                                    unsigned char num, AnimatedSpriteWarp *hit,
                                    std::optional<int> rotate,
                                    Skill *ski)
{
    std::vector<entt::entity> e;
    auto ent = owner;
    auto tr = World::registry->try_get<Transform>(ent);
    float step = 60 / (num + 1);
    for (size_t i = 0; i < num; i++)
    {
        Transform t(tr->position, tr->z, tr->flip);
        t.position.y = tr->position.y + 30 - step - i * step;
        e.push_back(load_ball(asprw, &t, ent, hit, rotate, ski));
    }
    return e;
}
