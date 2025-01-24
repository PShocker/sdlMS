#include "PlayerSkill.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 快速移动
int skill_2201002()
{
    auto ent = Player::ent;
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    // 添加effect
    auto eff = World::registry->try_get<Effect>(ent);
    eff->effects.push_back({new Transform(tr->position.x, tr->position.y), AnimatedSprite(Effect::load(u"BasicEff.img/Teleport"))});
    eff->effects.push_back({nullptr, AnimatedSprite(Effect::load(u"BasicEff.img/Teleport"))});

    auto x = tr->position.x;
    auto y = tr->position.y;

    if (Input::state[SDL_SCANCODE_RIGHT])
    {
        x += 200;
    }
    else if (Input::state[SDL_SCANCODE_LEFT])
    {
        x -= 200;
    }
    else if (Input::state[SDL_SCANCODE_DOWN])
    {
        y += 200;
    }
    else if (Input::state[SDL_SCANCODE_UP])
    {
        y -= 200;
    }
    if (x != tr->position.x || y != tr->position.y)
    {
        for (auto &e : World::registry->view<FloorFootHold>())
        {
            auto fh = World::registry->try_get<FootHold>(e);
            if (y != tr->position.y && fh == mv->foo)
            {
                continue;
            }
            if (fh->get_y(x).has_value())
            {
                auto fh_y = fh->get_y(x).value();
                if (y != tr->position.y)
                {
                    // y轴快速移动
                    auto y_min = std::min(tr->position.y, y);
                    auto y_max = std::max(tr->position.y, y);
                    if (fh_y <= y_min || fh_y >= y_max)
                    {
                        continue;
                    }
                }
                else
                {
                    // x轴快速移动
                    if (std::abs(fh_y - y) >= 100)
                    {
                        continue;
                    }
                }
                // 找到floor
                y = fh_y - 5;
                mv->foo = nullptr;
                if (!(cha->state == Character::State::ATTACK || cha->state == Character::State::SKILL))
                {
                    cha->state = Character::State::JUMP;
                    cha->action_index = 0;
                    cha->action_time = 0;
                    cha->action = Character::ACTION::JUMP;
                    cha->action_str = u"jump";
                }
                tr->position.x = x;
                tr->position.y = y;
                break;
            }
        }
    }
    mv->hspeed = 0;
    mv->vspeed = 0;

    // 技能音效
    PlayerSkill::skill_sound(SkillWarp::load(u"2201002"));

    SkillWarp::cooldowns[u"2201002"] = 500;
    player_portal_cooldown = 600;

    return PlayerSkill::SkillResult::None;
}