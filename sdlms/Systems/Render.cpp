#include "Render.h"
#include "entt/entt.hpp"
#include "Commons/Commons.h"
#include "Core/Core.h"

#include <SDL3/SDL.h>
#include <math.h>
#include <variant>
#include <optional>

void render_run()
{
    auto view = World::registry->view<Transform>();
    for (auto ent : view)
    {
        auto tr = &view.get<Transform>(ent);
        if (auto spr = World::registry->try_get<Sprite>(ent))
        {
            auto sprw = spr->sprw;
            render_sprite(tr, sprw);
        }
        else if (auto a = World::registry->try_get<AnimatedSprite>(ent))
        {
            render_animated_sprite(tr, a);
        }
        else if (auto bspr = World::registry->try_get<BackGround>(ent))
        {
            render_back_sprite(tr, bspr);
        }
        else if (auto por = World::registry->try_get<Portal>(ent))
        {
            render_portal(tr, por);
        }
        else if (auto npc = World::registry->try_get<Npc>(ent))
        {
            render_npc(tr, npc);
        }
        else if (auto mob = World::registry->try_get<Mob>(ent))
        {
            render_mob(tr, mob);
            if (auto eff = World::registry->try_get<Effect>(ent))
            {
                render_effect(tr, eff);
            }
            if (auto dam = World::registry->try_get<Damage>(ent))
            {
                render_damage(tr, dam);
            }
        }
        else if (auto cha = World::registry->try_get<Character>(ent))
        {
            if (auto tomb = World::registry->try_get<Tomb>(ent))
            {
                render_tomb(tomb);
            }
            if (auto i = World::registry->try_get<Install>(ent))
            {
                render_install(tr, i);
            }
            auto invincible_time = cha->invincible_cooldown;
            render_character(tr, cha, invincible_time);
            // SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 255);
            // auto rect = cha->r;
            // rect.x += tr->position.x - Camera::x;
            // rect.y += tr->position.y - Camera::y;
            // SDL_RenderFillRect(Window::renderer, &rect);
            if (auto aim = World::registry->try_get<AfterImage>(ent))
            {
                render_afterimage(tr, aim, cha);
            }
            if (auto eff = World::registry->try_get<Effect>(ent))
            {
                render_effect(tr, eff);
            }
            if (auto dam = World::registry->try_get<Damage>(ent))
            {
                render_damage(tr, dam);
            }
        }
        else if (auto dro = World::registry->try_get<Drop>(ent))
        {
            render_drop(tr, dro);
        }
        else if (auto r = World::registry->try_get<Reactor>(ent))
        {
            render_reactor(tr, r);
        }
        else if (auto sum = World::registry->try_get<Summon>(ent))
        {
            render_summon(tr, sum);
            if (auto eff = World::registry->try_get<Effect>(ent))
            {
                render_effect(tr, eff);
            }
        }
        else if (auto pet = World::registry->try_get<Pet>(ent))
        {
            render_pet(tr, pet);
            if (auto eff = World::registry->try_get<Effect>(ent))
            {
                render_effect(tr, eff);
            }
        }
    }
}

void render_sprite(Transform *tr, SpriteWarp *sprw, SDL_FPoint *o)
{
    float rot = tr->rotation;

    auto width = sprw->texture->w;
    auto heihgt = sprw->texture->h;

    auto x = tr->position.x;
    auto y = tr->position.y;

    SDL_FPoint origin;
    if (o != nullptr)
    {
        origin = *o;
    }
    else
    {
        origin.x = (float)sprw->origin.x;
        origin.y = (float)sprw->origin.y;
    }

    if (tr->camera == true)
    {
        // 显示坐标为绝对坐标,与摄像机无关,通常为ui
        SDL_FRect pos_rect{(float)x - origin.x, (float)y - origin.y, (float)width, (float)heihgt};
        SDL_RenderTextureRotated(Window::renderer, sprw->texture, nullptr, &pos_rect, rot, &origin, (SDL_FlipMode)tr->flip);
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
            pos_rect = {(float)x - (width - origin.x) - Camera::x, (float)y - origin.y - Camera::y, (float)width, (float)heihgt};
        }
        SDL_RenderTextureRotated(Window::renderer, sprw->texture, nullptr, &pos_rect, rot, &origin, (SDL_FlipMode)tr->flip);
    }
}

void render_animated_sprite(Transform *tr, AnimatedSprite *a, SDL_FPoint *o)
{
    auto sprw = a->asprw->sprites[a->anim_index];
    SDL_SetTextureAlphaMod(sprw->texture, a->alpha);
    render_sprite(tr, sprw, o);
}

void render_back_sprite(Transform *tr, BackGround *bspr)
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

    SpriteWarp *sprw = nullptr;

    if (std::holds_alternative<Sprite>(bspr->spr))
    {
        sprw = std::get<Sprite>(bspr->spr).sprw;
    }
    else
    {
        auto a = &std::get<AnimatedSprite>(bspr->spr);
        sprw = a->asprw->sprites[a->anim_index];
        SDL_SetTextureAlphaMod(sprw->texture, a->alpha);
    }
    spr_w = sprw->texture->w;
    spr_h = sprw->texture->h;
    spr_ox = sprw->origin.x;
    spr_oy = sprw->origin.y;

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

    Transform t(0, 0, 0, tr->flip);

    for (int i = 0; i < tile_cnt_y; i++)
    {
        for (int j = 0; j < tile_cnt_x; j++)
        {
            t.position = {(float)point.x + j * cx + spr_ox, (float)point.y + i * cy + spr_oy};
            render_sprite(&t, sprw);
        }
    }
}

void render_character(const Transform *tr, Character *cha, int invincible)
{
    auto action = cha->action;
    auto action_index = cha->action_index;
    auto alpha = cha->alpha;

    Transform transfrom(0, 0, 0, 0);

    const auto set_transform = [&tr, &transfrom](Transform &t, SpriteWarp *spr)
    {
        auto chara_pos = tr->position;
        auto flip = tr->flip;
        if (flip == 0)
        {
            transfrom.flip = 0;
            transfrom.position = chara_pos + t.position;
        }
        else
        {
            transfrom.flip = 1;
            auto x = -t.position.x;
            auto y = t.position.y;
            transfrom.position = chara_pos + SDL_FPoint{x, y};
        }
    };
    const auto render_avatar = [&set_transform, &transfrom, &action, &action_index, &invincible, &alpha](std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> part[Character::ACTION::LENGTH])
    {
        if (part[action].size() > 0)
        {
            auto &[t, sprw] = part[action][action_index];
            if (sprw != nullptr)
            {
                set_transform(t, sprw);
                if (invincible > 0)
                {
                    auto time = invincible % 200;
                    if (time > 100)
                    {
                        SDL_SetTextureColorMod(sprw->texture, 128, 128, 128); // 设置颜色调节为默认值
                    }
                    else
                    {
                        SDL_SetTextureColorMod(sprw->texture, 255, 255, 255); // 设置颜色调节为默认值
                    }
                }
                else
                {
                    SDL_SetTextureColorMod(sprw->texture, 255, 255, 255); // 设置颜色调节为默认值
                }
                SDL_SetTextureAlphaMod(sprw->texture, alpha);
                render_sprite(&transfrom, sprw);
            }
        }
    };
    render_avatar(cha->backTamingMobMid);
    render_avatar(cha->backMobEquipUnderSaddle);
    render_avatar(cha->backSaddle);
    render_avatar(cha->backMobEquipMid);
    render_avatar(cha->backTamingMobFront);
    render_avatar(cha->backMobEquipFront);
    render_avatar(cha->mobEquipRear);
    render_avatar(cha->tamingMobRear);
    render_avatar(cha->saddleRear);
    render_avatar(cha->characterEnd);
    render_avatar(cha->backWeapon);
    if (!cha->cap_vslot.contains(u"Hf"))
    {
        render_avatar(cha->backHairBelowHead);
    }
    render_avatar(cha->backShieldBelowBody);
    render_avatar(cha->backMailChestAccessory);
    render_avatar(cha->backCapAccessory);
    render_avatar(cha->backAccessoryFace);
    render_avatar(cha->backAccessoryEar);
    render_avatar(cha->backBody);
    render_avatar(cha->backGlove);
    render_avatar(cha->backGloveWrist);
    render_avatar(cha->backWeaponOverGlove);
    render_avatar(cha->backMailChestBelowPants);
    render_avatar(cha->backPantsBelowShoes);
    render_avatar(cha->backShoesBelowPants);
    render_avatar(cha->backPants);
    render_avatar(cha->backShoes);
    render_avatar(cha->backPantsOverShoesBelowMailChest);
    render_avatar(cha->backMailChest);
    render_avatar(cha->backPantsOverMailChest);
    render_avatar(cha->backMailChestOverPants);
    render_avatar(cha->backHead);
    render_avatar(cha->backAccessoryFaceOverHead);
    render_avatar(cha->backCape);
    if (!cha->cap_vslot.contains(u"H6"))
    {
        render_avatar(cha->backHairBelowCap);
    }
    if (!cha->cap_vslot.contains(u"H5"))
    {
        render_avatar(cha->backHairBelowCapNarrow);
    }
    if (!cha->cap_vslot.contains(u"H4"))
    {
        render_avatar(cha->backHairBelowCapWide);
    }
    render_avatar(cha->backWeaponOverHead);
    render_avatar(cha->backCap);
    if (!cha->cap_vslot.contains(u"H1"))
    {
        render_avatar(cha->backHair);
    }
    render_avatar(cha->backCapOverHair);
    render_avatar(cha->backShield);
    render_avatar(cha->backWeaponOverShield);
    render_avatar(cha->backWing);
    render_avatar(cha->backHairOverCape);
    render_avatar(cha->weaponBelowBody);
    if (!cha->cap_vslot.contains(u"Hb"))
    {
        render_avatar(cha->hairBelowBody);
    }
    render_avatar(cha->capeBelowBody);
    render_avatar(cha->shieldBelowBody);
    render_avatar(cha->capAccessoryBelowBody);
    render_avatar(cha->gloveBelowBody);
    render_avatar(cha->gloveWristBelowBody);
    render_avatar(cha->body);
    render_avatar(cha->gloveOverBody);
    render_avatar(cha->mailChestBelowPants);
    render_avatar(cha->pantsBelowShoes);
    render_avatar(cha->shoes);
    render_avatar(cha->pants);
    render_avatar(cha->mailChestOverPants);
    render_avatar(cha->shoesOverPants);
    render_avatar(cha->pantsOverShoesBelowMailChest);
    render_avatar(cha->shoesTop);
    render_avatar(cha->mailChest);
    render_avatar(cha->pantsOverMailChest);
    render_avatar(cha->mailChestOverHighest);
    render_avatar(cha->gloveWristOverBody);
    render_avatar(cha->mailChestTop);
    render_avatar(cha->weaponOverBody);
    render_avatar(cha->armBelowHead);
    render_avatar(cha->mailArmBelowHead);
    render_avatar(cha->armBelowHeadOverMailChest);
    render_avatar(cha->gloveBelowHead);
    render_avatar(cha->mailArmBelowHeadOverMailChest);
    render_avatar(cha->gloveWristBelowHead);
    render_avatar(cha->weaponOverArmBelowHead);
    render_avatar(cha->shield);
    render_avatar(cha->weapon);
    render_avatar(cha->arm);
    render_avatar(cha->hand);
    render_avatar(cha->glove);
    render_avatar(cha->mailArm);
    render_avatar(cha->gloveWrist);
    render_avatar(cha->cape);
    render_avatar(cha->head);
    if (!cha->cap_vslot.contains(u"Hs"))
    {
        render_avatar(cha->hairShade);
    }
    render_avatar(cha->accessoryFaceBelowFace);
    if (cha->show_face[action][action_index] == true)
    {
        render_avatar(cha->face);
    }
    render_avatar(cha->accessoryFaceOverFaceBelowCap);
    render_avatar(cha->capBelowAccessory);
    render_avatar(cha->accessoryEar);
    render_avatar(cha->capAccessoryBelowAccFace);
    render_avatar(cha->accessoryFace);
    render_avatar(cha->accessoryEyeShadow);
    render_avatar(cha->accessoryEye);
    if (!cha->cap_vslot.contains(u"H2"))
    {
        render_avatar(cha->hair);
    }
    render_avatar(cha->cap);
    render_avatar(cha->capAccessory);
    render_avatar(cha->accessoryEyeOverCap);
    if (!cha->cap_vslot.contains(u"H1"))
    {
        render_avatar(cha->hairOverHead);
    }
    render_avatar(cha->capOverHair);
    render_avatar(cha->weaponBelowArm);
    render_avatar(cha->armOverHairBelowWeapon);
    render_avatar(cha->mailArmOverHairBelowWeapon);
    render_avatar(cha->armOverHair);
    render_avatar(cha->gloveBelowMailArm);
    render_avatar(cha->mailArmOverHair);
    render_avatar(cha->gloveWristBelowMailArm);
    render_avatar(cha->weaponOverArm);
    render_avatar(cha->handBelowWeapon);
    render_avatar(cha->gloveBelowWeapon);
    render_avatar(cha->gloveWristBelowWeapon);
    render_avatar(cha->shieldOverHair);
    render_avatar(cha->weaponOverHand);
    render_avatar(cha->handOverHair);
    render_avatar(cha->gloveOverHair);
    render_avatar(cha->gloveWristOverHair);
    render_avatar(cha->weaponOverGlove);
    render_avatar(cha->capeOverHead);
    render_avatar(cha->weaponWristOverGlove);
    render_avatar(cha->emotionOverBody);
    render_avatar(cha->characterStart);
    render_avatar(cha->tamingMobMid);
    render_avatar(cha->mobEquipUnderSaddle);
    render_avatar(cha->saddleFront);
    render_avatar(cha->mobEquipMid);
    render_avatar(cha->tamingMobFront);
    render_avatar(cha->mobEquipFront);
}

void render_afterimage(Transform *tr, AfterImage *aft, Character *cha)
{
    if (aft->animate == true && aft->animated == false)
    {
        auto action = cha->action;
        auto aspr = aft->aspr;
        render_animated_sprite(tr, &aspr);
    }
}

void render_effect(Transform *tr, Effect *eff)
{
    for (auto &val : eff->effects)
    {
        if (val.delay <= Window::dt_now)
        {
            if (val.tr == nullptr)
            {
                render_animated_sprite(tr, &val.aspr);
            }
            else
            {
                render_animated_sprite(val.tr, &val.aspr);
            }
        }
    }
}

void render_portal(Transform *tr, Portal *por)
{
    // 渲染三段式传送门
    if (por->a.size() > 0)
    {
        if (auto ent = Player::ent; World::registry->valid(ent))
        {
            auto player_tr = World::registry->try_get<Transform>(ent);
            auto d_x = std::abs(player_tr->position.x - tr->position.x);
            auto d_y = std::abs(player_tr->position.y - tr->position.y);
            if (d_x <= 100 && d_y <= 100)
            {
                por->index = 0;
                render_animated_sprite(tr, &por->a[por->index]);
            }
            else if (d_x <= 150 && d_y <= 150)
            {
                por->index = 1;
                render_animated_sprite(tr, &por->a[por->index]);
            }
            else
            {
                por->index = -1;
            }
        }
    }
}

void render_npc(Transform *tr, Npc *npc)
{
    render_animated_sprite(tr, &npc->a[npc->index]);
}

void render_mob(Transform *tr, Mob *mob)
{
    auto a = &mob->a[mob->index];
    if (mob->state == Mob::State::REMOVE)
    {
        float alpha;
        if (mob->revive - Window::dt_now < mob->revive_alpha_time)
        {
            alpha = 1 - ((float)(mob->revive - Window::dt_now) / mob->revive_alpha_time);
        }
        else
        {
            alpha = 0;
        }
        a->alpha = alpha * 255;
    }
    auto sprw = a->asprw->sprites[a->anim_index];
    SDL_SetTextureColorMod(sprw->texture, mob->mod.r, mob->mod.g, mob->mod.b); // 设置颜色调节为默认值
    render_animated_sprite(tr, a);
}

void render_damage(Transform *tr, Damage *dam)
{
    for (auto it : dam->damages)
    {
        auto &info = it;
        if (info.delay < Window::dt_now)
        {
            auto p = info.point;
            int length = static_cast<int>(std::floor(std::log10(info.damage)) + 1);
            p.x = p.x - length * 34 + length * 8.5;
            int i = 0;
            while (info.damage > 0)
            {
                auto n = info.damage % 10;
                Transform transfrom(p);
                transfrom.position.x += (length - i) * 34 + info.x;
                transfrom.position.y -= 25.5 - info.alpha / 10 + info.y;
                switch (info.type)
                {
                case 0:
                    SDL_SetTextureAlphaMod(dam->red[n]->texture, info.alpha);
                    render_sprite(&transfrom, dam->red[n]);
                    break;
                case 1:
                    SDL_SetTextureAlphaMod(dam->violet[n]->texture, info.alpha);
                    render_sprite(&transfrom, dam->violet[n]);
                    break;
                case 2:
                    if (i == length - 1)
                    {
                        SDL_SetTextureAlphaMod(dam->cri[10]->texture, info.alpha);
                        render_sprite(&transfrom, dam->cri[10]);
                    }
                    SDL_SetTextureAlphaMod(dam->cri[n]->texture, info.alpha);
                    render_sprite(&transfrom, dam->cri[n]);
                    break;
                case 3:
                    SDL_SetTextureAlphaMod(dam->blue[n]->texture, info.alpha);
                    render_sprite(&transfrom, dam->blue[n]);
                    break;
                default:
                    break;
                }
                info.damage /= 10;
                i++;
            }
        }
    }
}

void render_tomb(Tomb *tomb)
{
    render_animated_sprite(&tomb->f, &tomb->aspr);
}

void render_drop(Transform *tr, Drop *dro)
{
    auto a = &dro->aspr;

    float alpha = 1;
    if (dro->destory - Window::dt_now < 900)
    {
        alpha = ((float)(dro->destory - Window::dt_now) / 900);
    }
    a->alpha = alpha * 255;

    auto sprw = a->asprw->sprites[a->anim_index];
    auto origin = SDL_FPoint{(float)sprw->texture->w / 2, (float)sprw->texture->h / 2};

    Transform tran(tr->position.x - (float)sprw->origin.x + (float)sprw->texture->w / 2,
                   tr->position.y - (float)sprw->origin.y + (float)sprw->texture->h / 2);
    tran.rotation = tr->rotation;
    render_animated_sprite(&tran, a, &origin);
}

void render_reactor(Transform *tr, Reactor *r)
{
    if (r->hit)
    {
        render_animated_sprite(tr, &r->a[r->index].hit);
    }
    else
    {
        render_animated_sprite(tr, &r->a[r->index].init);
    }
}

void render_install(Transform *tr, Install *i)
{
    Transform tran(tr->position, 0, tr->flip);
    auto aspr = i->aspr;
    auto spr = aspr.asprw->sprites[aspr.anim_index];
    tran.position.y -= (spr->texture->h - spr->origin.y);
    render_animated_sprite(&tran, &i->aspr);
}

void render_summon(Transform *tr, Summon *sum)
{
    auto a = &sum->a[sum->index];
    render_animated_sprite(tr, a);
}

void render_pet(Transform *tr, Pet *pet)
{
    auto a = &pet->a[pet->index];
    render_animated_sprite(tr, a);
}
