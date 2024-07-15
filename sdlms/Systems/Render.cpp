module;

#include "../Core/World.h"
#include "../Core/Window.h"
#include "../Core/Camera.h"
#include "../Common/Point.h"

#include "../Components/Sprite.h"
#include "../Components/Transform.h"
#include "../Components/AnimatedSprite.h"
#include "../Components/BackSprite.h"

module systems:render;

void Render::run()
{
    auto view = World::registry.view<Transform>();
    for (auto &ent : view)
    {
        auto tr = &view.get<Transform>(ent);
        if (auto spr = World::registry.try_get<Sprite>(ent))
        {
            render_sprite(tr, spr);
        }
        else if (auto aspr = World::registry.try_get<AnimatedSprite>(ent))
        {
            render_animated_sprite(tr, aspr);
        }
        else if (auto bspr = World::registry.try_get<BackSprite>(ent))
        {
            render_back_sprite(tr, bspr);
        }
    }
}

void Render::render_sprite(Transform *tr, Sprite *spr)
{
    float rot = tr->rotation;

    auto width = spr->width;
    auto heihgt = spr->height;

    auto x = tr->position.x;
    auto y = tr->position.y;

    const SDL_FPoint origin{(float)spr->origin.x, (float)spr->origin.y};
    if (tr->camera == true)
    {
        // 显示坐标为绝对坐标,与摄像机无关,通常为ui
        SDL_FRect pos_rect{(float)x - origin.x, (float)y - origin.y, (float)width, (float)heihgt};
        SDL_RenderTextureRotated(Window::renderer, spr->texture, nullptr, &pos_rect, rot, &origin, (SDL_FlipMode)tr->flip);
    }
    else
    {
        // 显示坐标与摄像机坐标相关
        SDL_FRect pos_rect;
        if (tr->flip == 0)
        {
            pos_rect = {(float)x - origin.x - Camera::x, (float)y - origin.y - Camera::y, (float)width, (float)heihgt};
        }
        else if (tr->flip == 1)
        {
            pos_rect = {(float)x - (spr->width - origin.x) - Camera::x, (float)y - origin.y - Camera::y, (float)width, (float)heihgt};
        }
        SDL_RenderTextureRotated(Window::renderer, spr->texture, nullptr, &pos_rect, rot, &origin, (SDL_FlipMode)tr->flip);
    }
}

void Render::render_animated_sprite(Transform *tr, AnimatedSprite *aspr)
{
    auto spr = aspr->sprites[aspr->anim_index];
    render_sprite(tr, spr);
}

void Render::render_back_sprite(Transform *tr, BackSprite *bspr)
{
    auto delta_time = Window::delta_time;

    auto viewprot_x = Camera::x;
    auto viewprot_y = Camera::y;
    auto viewprot_w = Camera::w;
    auto viewprot_h = Camera::h;

    auto spr_w = 0;
    auto spr_h = 0;

    float spr_ox = 0;
    float spr_oy = 0;

    Sprite *spr = nullptr;

    if (std::holds_alternative<Sprite *>(bspr->spr))
    {
        spr = std::get<Sprite *>(bspr->spr);
    }
    else
    {
        auto aspr = std::get<AnimatedSprite *>(bspr->spr);
        spr = aspr->sprites[aspr->anim_index];
    }
    spr_w = spr->width;
    spr_h = spr->height;
    spr_ox = spr->origin.x;
    spr_oy = spr->origin.y;

    int cx = bspr->cx;
    int cy = bspr->cy;

    if (cx == 0)
    {
        cx = spr_w;
    }
    if (cy == 0)
    {
        cy = spr_h;
    }

    if (bspr->hspeed)
    {
        bspr->offset_x = fmodf(bspr->offset_x + bspr->rx * 5 * delta_time / 1000.0, cx);
    }
    else
    {
        bspr->offset_x = (viewprot_x + viewprot_w / 2) * (bspr->rx + 100) / 100.0;
    }
    if (bspr->vspeed)
    {
        bspr->offset_y = fmodf(bspr->offset_y + bspr->ry * 5 * delta_time / 1000.0, cy);
    }
    else
    {
        bspr->offset_y = (viewprot_y + viewprot_h / 2) * (bspr->ry + 100) / 100.0;
    }

    SDL_FPoint point = tr->position - SDL_FPoint{spr_ox, spr_oy};

    point.x += bspr->offset_x;
    point.y += bspr->offset_y;

    auto tile_cnt_x = 1;
    if (bspr->htile && cx > 0)
    {
        auto tile_start_right = int(point.x + spr_w - viewprot_x) % cx;
        if (tile_start_right <= 0)
        {
            tile_start_right = tile_start_right + cx;
        }
        tile_start_right = tile_start_right + viewprot_x;

        auto tile_start_left = tile_start_right - spr_w;
        if (tile_start_left >= viewprot_x + viewprot_w)
        {
            tile_cnt_x = 0;
        }
        else
        {
            tile_cnt_x = ceil((viewprot_x + viewprot_w - tile_start_left) / float(cx));
            point.x = tile_start_left;
        }
    }

    auto tile_cnt_y = 1;
    if (bspr->vtile && cy > 0)
    {
        auto tile_start_bottom = int(point.y + spr_h - viewprot_y) % cy;
        if (tile_start_bottom <= 0)
        {
            tile_start_bottom = tile_start_bottom + cy;
        }
        tile_start_bottom = tile_start_bottom + viewprot_y;

        auto tile_start_top = tile_start_bottom - spr_h;
        if (tile_start_top >= viewprot_y + viewprot_h)
        {
            tile_cnt_y = 0;
        }
        else
        {
            tile_cnt_y = ceil((viewprot_y + viewprot_h - tile_start_top) / float(cy));
            point.y = tile_start_top;
        }
    }

    Transform *t = new Transform(0, 0, tr->flip);
    for (int i = 0; i < tile_cnt_y; i++)
    {
        for (int j = 0; j < tile_cnt_x; j++)
        {
            t->position = {(float)point.x + j * cx + spr_ox, (float)point.y + i * cy + spr_oy};
            render_sprite(t, spr);
        }
    }
    delete t;
}
