module;

#include <SDL3/SDL.h>
#include "entt/entt.hpp"

module systems;

import components;
import core;

void animate_run()
{
    auto view = World::registry->view<Animated>();
    for (auto &ent : view)
    {
        if (auto aspr = World::registry->try_get<AnimatedSprite>(ent))
        {
            animate_sprite(aspr);
        }
        else if (auto por = World::registry->try_get<Portal>(ent))
        {
            animate_portal(por);
        }
        else if (auto npc = World::registry->try_get<Npc>(ent))
        {
            animate_npc(npc);
            if (auto eff = World::registry->try_get<Effect>(ent))
            {
                animate_effect(eff);
            }
            if (auto dam = World::registry->try_get<Damage>(ent))
            {
                if (!animate_damage(dam))
                {
                    World::registry->remove<Damage>(ent);
                }
            }
        }
        else if (auto mob = World::registry->try_get<Mob>(ent))
        {
            animate_mob(mob);
            if (auto eff = World::registry->try_get<Effect>(ent))
            {
                animate_effect(eff);
            }
            if (auto dam = World::registry->try_get<Damage>(ent))
            {
                if (!animate_damage(dam))
                {
                    World::registry->remove<Damage>(ent);
                }
            }
        }
        else if (auto cha = World::registry->try_get<Character>(ent))
        {
            animate_character(cha);
            if (auto aft = World::registry->try_get<AfterImage>(ent))
            {
                if (aft->animated == false)
                {
                    auto cha = World::registry->try_get<Character>(ent);
                    animate_afterimage(aft, cha);
                }
            }
            if (auto eff = World::registry->try_get<Effect>(ent))
            {
                animate_effect(eff);
            }
        }
    }
}

bool animate_sprite(AnimatedSprite *a)
{
    bool r = true;
    if (a->animate)
    {
        auto delta_time = Window::delta_time;
        a->anim_time += delta_time;
        if (a->anim_time >= a->aspr->sprites[a->anim_index]->delay)
        {
            if (a->anim_index + a->anim_step == a->anim_size || a->anim_index + a->anim_step < 0)
            {
                if (a->aspr->z)
                {
                    if (a->anim_step == 1)
                    {
                        a->anim_index -= 1;
                        a->anim_step = -1;
                    }
                    else
                    {
                        a->anim_index += 1;
                        a->anim_step = 1;
                    }
                }
                else
                {
                    a->anim_index = 0;
                }
                a->anim_time = 0;
                r = false;
            }
            else
            {
                a->anim_index += a->anim_step;
                a->anim_time = 0;
            }
        }
        // 透明度处理
        auto a0 = a->aspr->sprites[a->anim_index]->a0;
        auto a1 = a->aspr->sprites[a->anim_index]->a1;
        a->alpha = a0;
        if (a0 != a1)
        {
            if (a0 <= a1)
            {
                a->alpha = (float)a0 + (float)(a1 - a0) / (float)a->aspr->sprites[a->anim_index]->delay * (float)a->anim_time;
            }
            else
            {
                a->alpha = (float)a0 - (float)(a0 - a1) / (float)a->aspr->sprites[a->anim_index]->delay * (float)a->anim_time;
            }
        }
    }
    return r;
}

void animate_character(Character *cha)
{
    if (cha->animate)
    {
        auto delta_time = Window::delta_time;
        auto delay = cha->stance_delays[cha->action][cha->action_index];
        cha->action_time += delta_time;
        if (cha->action_time >= delay)
        {
            if (cha->action_index == cha->stance_delays[cha->action].size() - 1)
            {
                cha->action_index = 0;
                cha->animated = true;
            }
            else
            {
                cha->action_index += 1;
            }
            cha->action_time = 0;
        }
    }
}

void animate_afterimage(AfterImage *aim, Character *cha)
{
    auto action = cha->action;
    uint8_t index = std::get<0>(aim->swordOS[u"0"][action]);
    auto aspr = std::get<1>(aim->swordOS[u"0"][action]);
    if (cha->action_index == index || aim->animate == true)
    {
        aim->animate = true;
        if (animate_sprite(aspr) == false)
        {
            aim->animated = true;
        }
    }
}

void animate_effect(Effect *eff)
{
    for (auto it = eff->effects.begin(); it != eff->effects.end();)
    {
        auto aspr = &(*it);
        if (animate_sprite(aspr) == false)
        {
            it = eff->effects.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return;
}

void animate_portal(Portal *por)
{
    // 更新三段式传送门,这里简单的更新三段式传送门的所有状态
    if (por->index >= 0)
    {
        animate_sprite(por->a[por->index]);
    }
}

void animate_npc(Npc *npc)
{
    if (!animate_sprite(npc->a[npc->index]))
    {
        // 切换npc状态
        auto it = npc->a.find(npc->index);
        auto next = std::next(it);
        if (next != npc->a.end())
        {
            npc->index = next->first;
        }
        else
        {
            npc->index = npc->a.begin()->first;
        }
    }
}

void animate_mob(Mob *mob)
{
    if (!animate_sprite(mob->a[mob->index]) && mob->state == Mob::State::HIT)
    {
        mob->state = Mob::State::STAND;
    }
}

bool animate_damage(Damage *dam)
{
    auto delta_time = Window::delta_time;
    dam->alpha -= Window::delta_time / 4;
    return dam->alpha > 0;
}
