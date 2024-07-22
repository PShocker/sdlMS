module;

#include <SDL3/SDL.h>
#include "entt/entt.hpp"

module systems;

import components;
import core;

void animate_run()
{
    {
        auto view = World::registry.view<AnimatedSprite>();
        for (auto &ent : view)
        {
            auto aspr = &view.get<AnimatedSprite>(ent);
            animate_sprite(aspr);
        }
    }
    {
        auto view = World::registry.view<Character>();
        for (auto &ent : view)
        {
            auto cha = &view.get<Character>(ent);
            animate_character(cha);
        }
    }
    {
        auto view = World::registry.view<AfterImage>();
        for (auto &ent : view)
        {
            auto aim = &view.get<AfterImage>(ent);
            if (aim->animated == false)
            {
                auto cha = World::registry.try_get<Character>(ent);
                animate_afterimage(aim, cha);
            }
        }
    }
}

bool animate_sprite(AnimatedSprite *aspr)
{
    bool r = true;
    if (aspr->animate)
    {
        auto delta_time = Window::delta_time;
        aspr->anim_time += delta_time;
        if (aspr->anim_time >= aspr->sprites[aspr->anim_index]->delay)
        {
            if (aspr->anim_index == aspr->anim_size - 1)
            {
                if (aspr->z)
                {
                    // 复杂度为O(n),需要重构
                    std::ranges::reverse(aspr->sprites);
                    aspr->anim_index = 1;
                }
                else
                {
                    aspr->anim_index = 0;
                }
                aspr->anim_time = 0;
                r = false;
            }
            else
            {
                aspr->anim_index += 1;
                aspr->anim_time = 0;
            }
        }
        // 透明度处理
        auto a0 = aspr->sprites[aspr->anim_index]->a0;
        auto a1 = aspr->sprites[aspr->anim_index]->a1;
        if (a0 != a1)
        {
            auto alpha = 255;
            if (a0 <= a1)
            {
                alpha = (float)a0 + (float)(a1 - a0) / (float)aspr->sprites[aspr->anim_index]->delay * (float)aspr->anim_time;
            }
            else
            {
                alpha = (float)a0 - (float)(a0 - a1) / (float)aspr->sprites[aspr->anim_index]->delay * (float)aspr->anim_time;
            }
            SDL_SetTextureAlphaMod(aspr->sprites[aspr->anim_index]->texture, alpha);
        }
        else
        {
            SDL_SetTextureAlphaMod(aspr->sprites[aspr->anim_index]->texture, a0);
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
