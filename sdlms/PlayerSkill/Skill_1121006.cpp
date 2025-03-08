#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Systems/Move.h"

// 突进
int skill_1121006(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    if (mv->foo == nullptr)
    {
        return PlayerSkill::SkillResult::None;
    }
    else
    {
        mv->hspeed = 0;
    }

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"1121006");

    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    auto attackCount = 1;
    auto mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    auto hit = nullptr;
    SoundWarp *souw = ski->skiw->sounds[u"Hit"];

    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 20);
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        auto o_mv = World::registry->try_get<Move>(ent);
        if (o_mv->foo)
        {
            auto o_tr = World::registry->try_get<Transform>(ent);
            o_mv->hspeed = o_tr->flip == 1 ? 100 : -100;
            o_mv->hforce = o_tr->flip == 1 ? 1400 : -1400;
            move_move(o_mv, o_tr, 800, 0.06, false);
            o_mv->hspeed = 0;
            auto o_cha = World::registry->try_get<Character>(ent);
            if (o_cha->invincible_cooldown <= 100)
            {
                o_cha->invincible_cooldown = 100;
            }
        }
    };

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}