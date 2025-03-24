#include "Ball.h"

#include "entt/entt.hpp"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

const std::unordered_set<std::u16string> track_no_skill =
    {
        u"4111005",
        u"2121003"};

entt::entity load_ball(entt::entity owner, SDL_FPoint point, int speed, Skill *ski)
{
    auto owner_tr = World::registry->try_get<Transform>(owner);

    auto ent = World::registry->create();
    auto ent_tr = &World::registry->emplace<Transform>(ent);

    ent_tr->position = owner_tr->position + point;
    ent_tr->flip = owner_tr->flip;

    auto count = World::registry->view<Ball>().size();
    ent_tr->z = owner_tr->z - count - 1;

    auto ball = &World::registry->emplace<Ball>(ent);
    ball->owner = owner;
    ball->hit = AnimatedSpriteWarp::load(Wz::Character->get_root()->find_from_path(u"Afterimage/hit.img/mace2"));
    ball->destory = Window::dt_now + (350 / (float)speed * 1000);

    AnimatedSpriteWarp *asprw = nullptr;
    if (ski != nullptr)
    {
        // 给ball附加技能特效
        auto ent_ski = &World::registry->emplace<Skill>(ent, ski->skiw->id);
        ent_ski->atk = ski->atk;
        if (track_no_skill.contains(ski->skiw->id))
        {
            // 这类技能不追踪怪物,水平直线移动,例如飞侠的多重飞镖
            ball->track = false;
            ball->track_hit = std::unordered_set<entt::entity>();
        }
        if (ski->skiw->hits.size() > 0)
        {
            ball->hit = ski->skiw->hits[0];
        }
        if (ski->skiw->node->get_child(u"ball") && ski->skiw->node->get_child(u"ball")->get_child(u"rotatePeriod"))
        {
            ball->rotate = dynamic_cast<wz::Property<int> *>(ski->skiw->node->get_child(u"ball")->get_child(u"rotatePeriod"))->get();
        }
        asprw = AnimatedSpriteWarp::load(ski->skiw->node->find_from_path(u"ball"));
    }
    if (asprw == nullptr)
    {
        auto weaponinfo = World::registry->try_get<WeaponInfo>(owner);
        std::u16string ball_path;
        if (weaponinfo->attack == WeaponInfo::Attack::BOW)
        {
            ball_path = u"Consume/0206.img/02060001/bullet";
        }
        else
        {
            ball_path = u"Consume/0207.img/02070005/bullet";
        }
        asprw = AnimatedSpriteWarp::load(Wz::Item->get_root()->find_from_path(ball_path));
    }

    World::registry->emplace<Animated>(ent);
    World::registry->emplace<AnimatedSprite>(ent, asprw);

    auto ent_mv = &World::registry->emplace<Move>(ent);

    ent_mv->hspeed = owner_tr->flip == 1 ? speed : -speed;
    ent_mv->vspeed = 0;

    World::zindex = true;
    return ent;
}
