module;

#include <SDL3/SDL.h>
#include "entt/entt.hpp"

module systems;

import components;
import core;

void animate_run()
{
    {
        auto view = World::registry->view<Animated>();
        for (auto &ent : view)
        {
            auto a = &view.get<Animated>(ent);
            animate_sprite(a);
        }
    }
    {
        auto view = World::registry->view<Character>();
        for (auto &ent : view)
        {
            auto cha = &view.get<Character>(ent);
            animate_character(cha);
        }
    }
    {
        auto view = World::registry->view<AfterImage>();
        for (auto &ent : view)
        {
            auto aim = &view.get<AfterImage>(ent);
            if (aim->animated == false)
            {
                auto cha = World::registry->try_get<Character>(ent);
                animate_afterimage(aim, cha);
            }
        }
    }
    {
        auto view = World::registry->view<Skill>();
        for (auto &ent : view)
        {
            auto ski = &view.get<Skill>(ent);
            animate_skill(ski);
        }
    }
    {
        auto view = World::registry->view<Portal>();
        for (auto &ent : view)
        {
            auto por = &view.get<Portal>(ent);
            if (por->a.size() > 0)
            {
                animate_portal(por);
            }
        }
    }
    {
        auto view = World::registry->view<Npc>();
        for (auto &ent : view)
        {
            auto npc = &view.get<Npc>(ent);
            animate_npc(npc);
        }
    }
    {
        auto view = World::registry->view<Mob>();
        for (auto &ent : view)
        {
            auto mob = &view.get<Mob>(ent);
            animate_mob(mob);
        }
    }
}

bool animate_sprite(Animated *a)
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

void animate_skill(Skill *ski)
{
    for (int i = 0; i < ski->animated.size(); i++)
    {
        if (ski->animated[i] == false)
        {
            auto aspr = ski->effects[ski->id][i];
            if (animate_sprite(aspr) == false)
            {
                ski->animated[i] = true;
            }
        }
    }
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
    animate_sprite(mob->a[mob->index]);
}
