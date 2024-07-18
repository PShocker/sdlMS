module;

#include <SDL3/SDL.h>
#include "entt/entt.hpp"

module systems;

import components;
import core;

void animate_run()
{
    auto view = World::registry.view<AnimatedSprite>();
    for (auto &ent : view)
    {
        auto aspr = &view.get<AnimatedSprite>(ent);
        animate_sprite(aspr);
    }
}

void animate_sprite(AnimatedSprite *aspr)
{
    if (aspr->animate)
    {
        auto delta_time = Window::delta_time;

        aspr->anim_time += delta_time;
        if (aspr->anim_time >= aspr->sprites[aspr->anim_index]->delay)
        {
            if (aspr->anim_index == aspr->anim_size - 1 && aspr->z)
            {
                // 复杂度为O(n),需要重构
                std::ranges::reverse(aspr->sprites);
                aspr->anim_index = 1;
            }
            else
            {
                aspr->anim_index = (aspr->anim_index + 1) % aspr->anim_size;
            }
            aspr->anim_time = 0;
        }
        // 透明度处理
        auto a0 = aspr->sprites[aspr->anim_index]->a0;
        auto a1 = aspr->sprites[aspr->anim_index]->a1;
        [[unlikely]]
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
}
