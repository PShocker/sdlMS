#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 金钱炸弹
int skill_4211006(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    // 通用攻击技能
    if (state == Character::State::CLIMB)
    {
        return PlayerSkill::SkillResult::None;
    }
    else if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 4211006);

    Skill::cooldowns[4211006] = Window::dt_now + 500;

    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    auto hit = ski->skiw->hits[0];
    auto mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    auto attackCount = 1;
    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw);
    ski->atk.value().damage = 0;

    auto view = World::registry->view<Drop>();
    for (auto e : view)
    {
        auto dr = &view.get<Drop>(e);
        if (dr->id == u"09000000")
        {
            auto d_tr = World::registry->try_get<Transform>(e);
            auto p_tr = World::registry->try_get<Transform>(ent);
            if (std::abs(p_tr->position.y - d_tr->position.y) <= 10)
            {
                if ((p_tr->flip == 1 && d_tr->position.x > p_tr->position.x && d_tr->position.x - p_tr->position.x < 200) ||
                    (p_tr->flip == 0 && d_tr->position.x < p_tr->position.x && p_tr->position.x - d_tr->position.x < 200))
                {
                    dr->destory = Window::dt_now + 700;
                    auto eff = World::registry->try_get<Effect>(ent);
                    eff->effects.emplace(4211006, Effect::Wrap{Transform(d_tr->position),
                                                                  AnimatedSprite(ski->skiw->hits[0]),
                                                                  Window::dt_now + 700});
                    ski->atk.value().damage += dr->nums;
                    ski->attack = true;
                    continue;
                }
            }
        }
    }
    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT | PlayerSkill::SkillResult::ALERT;
}