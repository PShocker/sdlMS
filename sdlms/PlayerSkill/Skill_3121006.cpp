#include "PlayerSkill.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "Entities/Entities.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Common.h"

// 火凤凰
int skill_3121006(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    if (mv->foo == nullptr || ent != Player::ent)
    {
        return PlayerSkill::SkillResult::None;
    }
    else
    {
        mv->hspeed = 0;
    }

    // 持续时间
    const unsigned int duration = 30000;

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 3121006);
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 0))
        {
            return;
        }

        auto buff = World::registry->try_get<Buff>(ent);
        auto owner_mv = World::registry->try_get<Move>(ent);
        if (owner_mv->foo)
        {
            for (auto e : World::registry->view<Summon>())
            {
                auto sum = World::registry->try_get<Summon>(e);
                if (sum->state != Summon::State::DIE && sum->owner == ent && sum->id == 3121006)
                {
                    sum->destory = Window::dt_now + duration;
                    buff->buffs.at(3121006).destory = sum->destory;
                    return;
                }
            }
            Skill *ski = World::registry->try_get<Skill>(ent);
            auto sum_ent = load_summon(ski->skiw->node->get_child(u"summon"), 3121006, ent);
            auto sum = World::registry->try_get<Summon>(sum_ent);
            sum->atk.call_back = [n = ski->skiw->node](entt::entity src, entt::entity target, int full_damage)
            {
                auto tr = World::registry->try_get<Transform>(target);
                auto eff = World::registry->try_get<Effect>(target);

                for (int i = tr->position.x - 75; i <= tr->position.x + 75;)
                {
                    Effect::Wrap wrap;
                    wrap.tr = Transform(SDL_FPoint{(float)i, (float)tr->position.y});
                    AnimatedSprite::Wrap *asprw;
                    int random = std::rand() % 3;
                    switch (random)
                    {
                    case 0:
                        asprw = AnimatedSprite::Wrap::load(n->find_from_path(u"tile/0"));
                        break;
                    case 1:
                        asprw = AnimatedSprite::Wrap::load(n->find_from_path(u"tile/1"));
                        break;
                    case 2:
                        asprw = AnimatedSprite::Wrap::load(n->find_from_path(u"tile/2"));
                        break;
                    }
                    wrap.aspr = AnimatedSprite(asprw);
                    wrap.destory = Window::dt_now + 960;
                    eff->effects.emplace(0, wrap);
                    i += asprw->sprites[0]->texture->w;
                }
                const auto mob = World::registry->try_get<Mob>(target);
                unsigned int time = 6000;
                mob->call_backs.erase(3121006);
                mob->call_backs.emplace(3121006, std::make_pair(flame_call_back, std::make_tuple(Window::dt_now + time, Window::dt_now + 500)));
            };
            Buff::Wrap wrap;
            wrap.duration = duration;
            wrap.destory = Window::dt_now + duration;
            buff->buffs.emplace(3121006, wrap);
        }
    };
    Skill::cooldowns[3121006] = Window::dt_now + 1500;
    return PlayerSkill::SkillResult::SOU | PlayerSkill::SkillResult::ACT;
}