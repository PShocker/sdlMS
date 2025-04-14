#include "Ball.h"

#include "entt/entt.hpp"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

const std::unordered_set<int> track_no_skill =
    {4111005, 2121003};

entt::entity load_ball(entt::entity owner, SDL_FPoint point, int speed, Skill *ski)
{
    auto owner_tr = World::registry->try_get<Transform>(owner);

    auto ent = World::registry->create();
    auto ent_tr = &World::registry->emplace<Transform>(ent);

    ent_tr->position = point;
    ent_tr->flip = owner_tr->flip;

    auto count = World::registry->view<Ball>().size();
    ent_tr->z = owner_tr->z - count - 1;

    auto ball = &World::registry->emplace<Ball>(ent);
    ball->point = point;
    ball->owner = owner;
    ball->hit = AnimatedSprite::Wrap::load(Wz::Character->get_root()->find_from_path(u"Afterimage/hit.img/mace2"));
    ball->destory = Window::dt_now + (350 / (float)speed * 1000);

    AnimatedSprite::Wrap *asprw = nullptr;
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
        asprw = AnimatedSprite::Wrap::load(ski->skiw->node->find_from_path(u"ball"));
    }
    if (asprw == nullptr)
    {
        auto weaponWrap = World::registry->try_get<WeaponWrap>(owner);
        std::u16string ball_path;
        if (weaponWrap->attack == WeaponWrap::Attack::BOW)
        {
            asprw = AnimatedSprite::Wrap::load(Skill(3201004).skiw->node->find_from_path(u"ball"));
        }
        else
        {
            asprw = AnimatedSprite::Wrap::load(Wz::Item->get_root()->find_from_path(u"Consume/0207.img/02070006/bullet"));
        }
    }

    World::registry->emplace<AnimatedSprite>(ent, asprw);

    auto ent_mv = &World::registry->emplace<Move>(ent);

    ent_mv->hspeed = owner_tr->flip == 1 ? speed : -speed;
    ent_mv->vspeed = 0;

    World::zindex = true;
    return ent;
}
