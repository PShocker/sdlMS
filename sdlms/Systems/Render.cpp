#include "Render.h"
#include "entt/entt.hpp"
#include "Commons/Commons.h"
#include "Core/Core.h"
#include "Systems/UI.h"
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
            render_sprite(tr, spr->sprw);
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
            if (auto nametag = World::registry->try_get<NameTag>(ent))
            {
                render_nametag(tr, nametag);
            }
        }
        else if (auto npc = World::registry->try_get<Npc>(ent))
        {
            render_npc(tr, npc);
            if (auto nametag = World::registry->try_get<NameTag>(ent))
            {
                render_nametag(tr, nametag);
            }
        }
        else if (auto mob = World::registry->try_get<Mob>(ent))
        {
            render_mob(tr, mob);
            if (auto eff = World::registry->try_get<Effect>(ent))
            {
                render_effect(tr, eff);
            }
            if (auto nametag = World::registry->try_get<NameTag>(ent))
            {
                if (mob->state == Mob::State::REMOVE)
                {
                    if (mob->revive - Window::dt_now < mob->revive_alpha_time)
                    {
                        render_nametag(tr, nametag);
                    }
                }
                else
                {
                    render_nametag(tr, nametag);
                }
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
            if (auto eff = World::registry->try_get<Effect>(ent))
            {
                render_effect_front(tr, eff);
            }
            auto invincible_time = cha->invincible_cooldown - Window::dt_now;
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
                render_effect_back(tr, eff);
            }
            if (auto nametag = World::registry->try_get<NameTag>(ent))
            {
                render_nametag(tr, nametag);
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
            if (auto nametag = World::registry->try_get<NameTag>(ent))
            {
                render_nametag(tr, nametag);
            }
        }
    }
    // 对于和Layer无关的属性，需要单独处理渲染
    render_damage();
    render_chatballoon();
    // 对于UI逻辑，需要单独处理渲染逻辑，不需要添加Transform组件排序
    render_gaintip();
    render_ui();
    render_cursor();
    render_tooltip();
}

void render_texture(SDL_Texture *texture, SDL_FRect *src_rect, SDL_FRect *pos_rect, int alpha, int flip, float rotation, SDL_FPoint *origin)
{
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderTextureRotated(Window::renderer, texture, src_rect, pos_rect, rotation, origin, (SDL_FlipMode)flip);
}

void render_sprite(SDL_FPoint &p, Sprite::Wrap *sprw, int flip, float rotation, SDL_FPoint *origin, int alpha)
{
    auto width = sprw->texture->w;
    auto heihgt = sprw->texture->h;
    SDL_FPoint o{(float)sprw->origin.x, (float)sprw->origin.y};
    origin = origin == nullptr ? &o : origin;
    SDL_FRect pos_rect;
    if (flip == 0)
    {
        pos_rect = {(float)p.x - origin->x, (float)p.y - origin->y, (float)width, (float)heihgt};
    }
    else if (flip == 1)
    {
        pos_rect = {(float)p.x - (width - origin->x), (float)p.y - origin->y, (float)width, (float)heihgt};
    }
    render_texture(sprw->texture, nullptr, &pos_rect, alpha, flip, rotation, origin);
}

void render_sprite(Transform *tr, Sprite::Wrap *sprw, SDL_FPoint *origin, int alpha)
{
    float rot = tr->rotation;
    SDL_FPoint position = tr->position - SDL_FPoint{(float)Camera::x, (float)Camera::y};
    render_sprite(position, sprw, tr->flip, rot, origin, alpha);
}

void render_animated_sprite(SDL_FPoint &p, AnimatedSprite *a, int alpha)
{
    auto sprw = a->asprw->sprites[a->anim_index];
    render_sprite(p, sprw, 0, 0, nullptr, a->alpha * ((float)alpha / 255));
}

void render_animated_sprite(Transform *tr, AnimatedSprite *a, SDL_FPoint *origin, int alpha)
{
    auto sprw = a->asprw->sprites[a->anim_index];
    render_sprite(tr, sprw, origin, a->alpha * ((float)alpha / 255));
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

    Sprite::Wrap *sprw = nullptr;
    int alpha = 255;
    if (std::holds_alternative<Sprite>(bspr->spr))
    {
        sprw = std::get<Sprite>(bspr->spr).sprw;
    }
    else
    {
        auto a = &std::get<AnimatedSprite>(bspr->spr);
        sprw = a->asprw->sprites[a->anim_index];
        alpha = a->alpha;
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
            render_sprite(&t, sprw, nullptr, alpha);
        }
    }
}

void render_character(const Transform *tr, Character *cha, int invincible)
{
    auto action = cha->action;
    auto action_index = cha->action_index;
    auto alpha = cha->alpha;

    Transform transfrom(0, 0, 0, 0);

    const auto set_transform = [&tr, &transfrom](Transform &t, Sprite::Wrap *spr)
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
    const auto render_avatar = [&set_transform, &transfrom, &action, &action_index, &invincible, &alpha](std::unordered_map<uint8_t, std::pair<Transform, Sprite::Wrap *>> part[Character::Action::LENGTH])
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
                        SDL_SetTextureColorMod(sprw->texture, 128, 128, 128);
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
                render_sprite(&transfrom, sprw, nullptr, alpha);
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

void render_effect_front(Transform *tr, Effect *eff)
{
    for (auto &[key, val] : eff->effects)
    {
        if (val.delay <= Window::dt_now)
        {
            if (key < 0)
            {
                if (val.tr == std::nullopt)
                {
                    render_animated_sprite(tr, &val.aspr);
                }
                else if (val.follow)
                {
                    Transform tran(val.tr.value().position + tr->position, 0, tr->flip);
                    render_animated_sprite(&tran, &val.aspr);
                }
                else
                {
                    render_animated_sprite(&val.tr.value(), &val.aspr);
                }
            }
        }
    }
}

void render_effect(Transform *tr, Effect *eff)
{
    for (auto &[key, val] : eff->effects)
    {
        if (val.delay <= Window::dt_now)
        {
            if (val.tr == std::nullopt)
            {
                render_animated_sprite(tr, &val.aspr);
            }
            else if (val.follow)
            {
                Transform tran(val.tr.value().position + tr->position, 0, tr->flip);
                render_animated_sprite(&tran, &val.aspr);
            }
            else if (val.target)
            {
                // 打击点特效
                auto sprw = val.aspr.asprw->sprites[val.aspr.anim_index];
                SDL_FPoint p{(float)sprw->origin.x - sprw->texture->w / 2, (float)sprw->origin.y - sprw->texture->h / 2};
                Transform tran(val.tr.value().position + p, 0, tr->flip);
                render_animated_sprite(&tran, &val.aspr);
            }
            else
            {
                // 固定位置特效
                render_animated_sprite(&val.tr.value(), &val.aspr);
            }
        }
    }
}

void render_effect_back(Transform *tr, Effect *eff)
{
    for (auto &[key, val] : eff->effects)
    {
        if (val.delay <= Window::dt_now)
        {
            if (key < 0)
            {
                continue;
            }
            if (val.tr == std::nullopt)
            {
                render_animated_sprite(tr, &val.aspr);
            }
            else if (val.follow)
            {
                Transform tran(val.tr.value().position + tr->position, 0, tr->flip);
                render_animated_sprite(&tran, &val.aspr);
            }
            else
            {
                render_animated_sprite(&val.tr.value(), &val.aspr);
            }
        }
    }
}

void render_portal(Transform *tr, Portal *por)
{
    // 渲染三段式传送门
    if (por->a.size() > 0 && por->index >= 0)
    {
        render_animated_sprite(tr, &por->a[por->index]);
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

void render_damage()
{
    for (auto ent : World::registry->view<Damage>())
    {
        auto dam = World::registry->try_get<Damage>(ent);
        auto d = dam->damages;
        d.sort([](const auto &m, const auto &n)
               { return m.alpha < n.alpha; });
        for (auto it : d)
        {
            auto &wrap = it;
            if (wrap.delay <= Window::dt_now)
            {
                auto p = wrap.point;
                int length = static_cast<int>(std::floor(std::log10(wrap.damage)) + 1);
                p.x = p.x - length * 14;
                int i = 0;
                while (wrap.damage > 0)
                {
                    auto n = wrap.damage % 10;
                    Transform transfrom(p);
                    transfrom.position.x += (length - i) * 28;
                    transfrom.position.y -= 25.5 - wrap.alpha / 10;
                    switch (wrap.type)
                    {
                    case 0:
                        render_sprite(&transfrom, dam->red[n], nullptr, wrap.alpha);
                        break;
                    case 1:
                        render_sprite(&transfrom, dam->violet[n], nullptr, wrap.alpha);
                        break;
                    case 2:
                        if (i == length - 1)
                        {
                            render_sprite(&transfrom, dam->cri[10], nullptr, wrap.alpha);
                        }
                        render_sprite(&transfrom, dam->cri[n], nullptr, wrap.alpha);
                        break;
                    case 3:
                        render_sprite(&transfrom, dam->blue[n], nullptr, wrap.alpha);
                        break;
                    default:
                        break;
                    }
                    wrap.damage /= 10;
                    i++;
                }
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
    float alpha = 255;
    if (dro->destory - Window::dt_now < 900)
    {
        alpha = ((float)(dro->destory - Window::dt_now) / 900) * 255;
    }
    Sprite::Wrap *sprw;
    if (std::holds_alternative<Sprite>(dro->spr))
    {
        sprw = std::get<Sprite>(dro->spr).sprw;
    }
    else
    {
        auto a = &std::get<AnimatedSprite>(dro->spr);
        sprw = a->asprw->sprites[a->anim_index];
    }
    auto origin = SDL_FPoint{(float)sprw->texture->w / 2, (float)sprw->texture->h / 2};
    Transform tran(tr->position.x - (float)sprw->origin.x + (float)sprw->texture->w / 2,
                   tr->position.y - (float)sprw->origin.y + (float)sprw->texture->h / 2);
    tran.rotation = tr->rotation;
    render_sprite(&tran, sprw, &origin, alpha);
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

void render_uibuff()
{
    for (auto ent : World::registry->view<UIBuff>())
    {
        auto uib = World::registry->try_get<UIBuff>(ent);
        auto position = &uib->position;
        auto sprw = World::registry->try_get<Sprite>(ent)->sprw;
        render_sprite(*position, sprw);
        if (uib->destory >= Window::dt_now && uib->duration > 0)
        {
            SDL_SetRenderDrawBlendMode(Window::renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 148);
            auto d = uib->destory - Window::dt_now;
            SDL_FRect pos_rect;
            pos_rect.x = position->x;
            pos_rect.y = position->y - 32 + 32 * d / (float)uib->duration;
            pos_rect.w = 32;
            pos_rect.h = 32 * (1 - d / (float)uib->duration);
            SDL_RenderFillRect(Window::renderer, &pos_rect);
            // 渲染冷却时间
            int num = d / 1000;
            // 计算数字位数
            int digits = num == 0 ? 1 : static_cast<int>(std::log10(num)) + 1;
            // 从最高位开始遍历
            int w = 0;
            for (int i = digits - 1; i >= 0; --i)
            {
                int divisor = static_cast<int>(std::pow(10, i));
                int digit = (num / divisor) % 10;
                pos_rect = {(float)position->x + w, (float)position->y - UIBuff::number_cache[digit]->h,
                            (float)UIBuff::number_cache[digit]->w, (float)UIBuff::number_cache[digit]->h};
                SDL_RenderTexture(Window::renderer, UIBuff::number_cache[digit], nullptr, &pos_rect);
                w += UIBuff::number_cache[digit]->w;
            }
        }
    }
}

void render_nametag(Transform *tr, NameTag *nametag)
{
    auto h = 0;
    for (int i = 0; i < nametag->nametags.size(); i++)
    {
        auto str_texture = nametag->nametags[i].str_texture;

        // 先渲染背景
        auto back_texture = nametag->nametags[i].back_texture;
        if (back_texture == nullptr)
        {
            SDL_SetRenderDrawBlendMode(Window::renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 178);
            SDL_FRect rect;
            rect.w = str_texture->w + 4;
            rect.h = str_texture->h + 4;
            rect.x = tr->position.x - Camera::x - rect.w / 2 + nametag->offset_x;
            rect.y = tr->position.y - Camera::y + h + i * 2 + nametag->offset_y;
            SDL_RenderFillRect(Window::renderer, &rect);
        }
        auto pos_rect = SDL_FRect{tr->position.x - Camera::x - str_texture->w / 2 + nametag->offset_x,
                                  i * 2 + h + tr->position.y - Camera::y + nametag->offset_y,
                                  (float)str_texture->w,
                                  (float)str_texture->h};
        SDL_RenderTexture(Window::renderer, str_texture, nullptr, &pos_rect);
        h += str_texture->h;
    }
}

void render_chatballoon()
{
    for (auto ent : World::registry->view<ChatBalloon>())
    {
        auto chatballoon = World::registry->try_get<ChatBalloon>(ent);
        auto tr = World::registry->try_get<Transform>(ent);

        auto &it = chatballoon->chatballoons[0];
        if (it.delay <= Window::dt_now)
        {
            it.delay = Window::dt_now + 5000 + std::rand() % 15000;
            it.show = !it.show;
        }
        if (it.show == false)
        {
            continue;
        }
        auto str_texture = chatballoon->chatballoons[0].str_texture;
        // 先渲染背景
        auto back_texture = chatballoon->chatballoons[0].back_texture;

        float render_back_y = 0;
        if (auto npc = World::registry->try_get<Npc>(ent))
        {
            auto aspr = npc->a[npc->index];
            auto sprw = aspr.asprw->sprites[aspr.anim_index];
            render_back_y = (float)-sprw->texture->h - (sprw->origin.y - sprw->texture->h) - (float)back_texture->h;
        }

        auto pos_rect = SDL_FRect{tr->position.x - Camera::x - back_texture->w / 2, tr->position.y - Camera::y + render_back_y, (float)back_texture->w, (float)back_texture->h};
        SDL_RenderTexture(Window::renderer, back_texture, nullptr, &pos_rect);
        // 文字的起始高度为渲染背景高度加上（背景高度减去文字高度/2）
        pos_rect = SDL_FRect{tr->position.x - Camera::x - str_texture->w / 2, tr->position.y - Camera::y + render_back_y + (back_texture->h - 7 - str_texture->h) / 2, (float)str_texture->w, (float)str_texture->h};
        SDL_RenderTexture(Window::renderer, str_texture, nullptr, &pos_rect);
    }
}

void render_statusbar()
{
    // 渲染backgrnd3
    SDL_FRect pos_rect{(float)0, (float)Camera::h - StatusBar::backgrnd3->h, (float)StatusBar::backgrnd3->w, (float)StatusBar::backgrnd3->h};
    render_texture(StatusBar::backgrnd3, nullptr, &pos_rect, StatusBar::alpha);
    // 渲染按钮
    for (auto &[key, val] : StatusBar::position_map)
    {
        auto aspr = key->second.at(key->first);
        auto position = SDL_FPoint{(float)val.x + aspr.asprw->sprites[aspr.anim_index]->origin.x, (float)Camera::h + val.y + aspr.asprw->sprites[aspr.anim_index]->origin.y};
        render_animated_sprite(position, &aspr, StatusBar::alpha);
    }
    // 聊天栏
    if (!StatusBar::chatOpen)
    {
        auto texture = StatusBar::chats[0];
        pos_rect = SDL_FRect{(float)6, (float)Camera::h - StatusBar::backgrnd3->h + (24 - texture->h), (float)texture->w, (float)texture->h};
        render_texture(texture, nullptr, &pos_rect, StatusBar::alpha);
    }
}

void render_worldmap()
{

    auto sprw = WorldMap::baseimg.spr.sprw;
    SDL_FRect pos_rect = {(float)WorldMap::x - sprw->origin.x - 6,
                          (float)WorldMap::y - sprw->origin.y - 28,
                          (float)WorldMap::backgrnd->w, (float)WorldMap::backgrnd->h};
    render_texture(WorldMap::backgrnd, nullptr, &pos_rect, 255);

    auto position = SDL_FPoint{WorldMap::x, WorldMap::y};
    render_sprite(position, sprw);

    if (WorldMap::cur_link)
    {
        SDL_FPoint p = position;
        render_sprite(p, WorldMap::cur_link->spr.sprw);
    }

    for (auto &it : WorldMap::spots)
    {
        auto p = position + it.p;
        render_sprite(p, it.spr.sprw);
    }
    if (WorldMap::curpos.p.has_value())
    {
        auto aspr = WorldMap::curpos.aspr;
        auto p = position + WorldMap::curpos.p.value();
        render_animated_sprite(p, &aspr);
    }

    for (auto &[key, val] : WorldMap::position_map)
    {
        auto aspr = key->second.at(key->first);
        auto position = SDL_FPoint{(float)WorldMap::x - sprw->origin.x - 6 + WorldMap::backgrnd->w + val.x,
                                   (float)WorldMap::y - sprw->origin.y - 28 + val.y};
        render_animated_sprite(position, &aspr, 255);
    }
}

void render_keyconfig()
{
    SDL_FRect pos_rect = {(float)KeyConfig::x, (float)KeyConfig::y, (float)KeyConfig::backgrnd->w, (float)KeyConfig::backgrnd->h};
    render_texture(KeyConfig::backgrnd, nullptr, &pos_rect, KeyConfig::alpha);

    for (auto &[key, val] : KeyConfig::position_map)
    {
        auto aspr = key->second.at(key->first);
        auto position = SDL_FPoint{(float)KeyConfig::x + val.x + aspr.asprw->sprites[aspr.anim_index]->origin.x, (float)KeyConfig::y + val.y + aspr.asprw->sprites[aspr.anim_index]->origin.y};
        render_animated_sprite(position, &aspr, KeyConfig::alpha);
    }
}

void render_uiitem()
{
    SDL_FRect pos_rect = {(float)UIItem::x, (float)UIItem::y, (float)UIItem::FullBackgrnd->w, (float)UIItem::FullBackgrnd->h};
    render_texture(UIItem::FullBackgrnd, nullptr, &pos_rect, UIItem::alpha);

    // 渲染tab
    for (int i = 0; i <= 4; i++)
    {
        SDL_Texture *texture;
        if (i == UIItem::active_tab)
        {
            texture = UIItem::tabs[i];
        }
        else
        {
            texture = UIItem::tabs[i + 5];
        }
        pos_rect = {(float)UIItem::x + 4 + i * texture->w, (float)UIItem::y + 23, (float)texture->w, (float)texture->h};
        render_texture(texture, nullptr, &pos_rect, UIItem::alpha);
    }
    // 渲染物品
    for (int i = 0; i < 96; i++)
    {
        auto sprw = UIItem::items[UIItem::active_tab][i].sprw;
        if (sprw != nullptr)
        {
            auto page = i / 24;
            auto index = i % 4;
            auto line = i / 4;
            line = line % 6;
            SDL_FPoint p = {(float)UIItem::x + 8 + index * 36 + page * 148, (float)UIItem::y + 82 + line * 34};
            render_sprite(p, sprw, 0, 0, nullptr, UIItem::alpha);
        }
    }

    for (auto &[key, val] : UIItem::position_map)
    {
        auto aspr = key->second.at(key->first);
        auto position = SDL_FPoint{(float)UIItem::x + val.x + aspr.asprw->sprites[aspr.anim_index]->origin.x, (float)UIItem::y + val.y + aspr.asprw->sprites[aspr.anim_index]->origin.y};
        render_animated_sprite(position, &aspr, UIItem::alpha);
    }
}

void render_uiskill()
{
    SDL_FRect pos_rect = {(float)UISkill::x, (float)UISkill::y, (float)UISkill::backgrnd->w, (float)UISkill::backgrnd->h};
    render_texture(UISkill::backgrnd, nullptr, &pos_rect, UISkill::alpha);

    for (auto &[key, val] : UISkill::position_map)
    {
        auto aspr = key->second.at(key->first);
        auto position = SDL_FPoint{(float)UISkill::x + val.x + aspr.asprw->sprites[aspr.anim_index]->origin.x, (float)UISkill::y + val.y + aspr.asprw->sprites[aspr.anim_index]->origin.y};
        render_animated_sprite(position, &aspr, UISkill::alpha);
    }
}

void render_uistat()
{
    SDL_FRect pos_rect = {(float)UIStat::x, (float)UIStat::y, (float)UIStat::backgrnd->w, (float)UIStat::backgrnd->h};
    render_texture(UIStat::backgrnd, nullptr, &pos_rect, UIStat::alpha);

    for (auto &[key, val] : UIStat::position_map)
    {
        auto aspr = key->second.at(key->first);
        auto position = SDL_FPoint{(float)UIStat::x + val.x + aspr.asprw->sprites[aspr.anim_index]->origin.x, (float)UIStat::y + val.y + aspr.asprw->sprites[aspr.anim_index]->origin.y};
        render_animated_sprite(position, &aspr, UIStat::alpha);
    }
}

void render_minimap()
{
    if (!MiniMap::minimize)
    {
        SDL_FRect pos_rect = {(float)MiniMap::x, (float)MiniMap::y, (float)MiniMap::backgrnd->w, (float)MiniMap::backgrnd->h};
        render_texture(MiniMap::backgrnd, nullptr, &pos_rect, MiniMap::alpha);

        pos_rect = {(float)MiniMap::x + (MiniMap::backgrnd->w - MiniMap::canvas->w) / 2, (float)MiniMap::y + 72, (float)MiniMap::canvas->w, (float)MiniMap::canvas->h};
        render_texture(MiniMap::canvas, nullptr, &pos_rect, MiniMap::alpha);

        // 绘制小地图人物，传送门，npc
        for (auto &[r, texture, ent] : MiniMap::points)
        {
            render_texture(texture, nullptr, &r, MiniMap::alpha);
        }
    }
    else
    {
        // 最小化
        SDL_FRect src_rect = {(float)0, (float)0, (float)MiniMap::backgrnd->w, (float)24};
        SDL_FRect pos_rect = {(float)MiniMap::x, (float)MiniMap::y, (float)MiniMap::backgrnd->w, (float)24};
        render_texture(MiniMap::backgrnd, &src_rect, &pos_rect, MiniMap::alpha);
    }
    for (auto &[key, val] : MiniMap::position_map)
    {
        auto aspr = key->second.at(key->first);
        auto position = SDL_FPoint{(float)val.x + MiniMap::x + MiniMap::backgrnd->w, (float)val.y + MiniMap::y};
        render_animated_sprite(position, &aspr, MiniMap::alpha);
    }
}

void render_uiequip()
{
    SDL_FRect pos_rect = {(float)UIEquip::x, (float)UIEquip::y, (float)UIEquip::backgrnd->w, (float)UIEquip::backgrnd->h};
    render_texture(UIEquip::backgrnd, nullptr, &pos_rect, UIEquip::alpha);

    for (auto &[key, val] : UIEquip::position_map)
    {
        auto aspr = key->second.at(key->first);
        auto position = SDL_FPoint{(float)UIEquip::x + val.x + aspr.asprw->sprites[aspr.anim_index]->origin.x, (float)UIEquip::y + val.y + aspr.asprw->sprites[aspr.anim_index]->origin.y};
        render_animated_sprite(position, &aspr, UIEquip::alpha);
    }
}

void render_cursor()
{
    SDL_FPoint p{Cursor::x, Cursor::y};
    render_animated_sprite(p, &Cursor::a[Cursor::index]);
}

void render_tooltip()
{
    for (auto &[r, texture] : ToolTip::tooltips)
    {
        SDL_SetRenderDrawBlendMode(Window::renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 178);
        SDL_FRect rect;
        rect.w = texture->w + 4;
        rect.h = texture->h + 4;
        rect.x = r.x;
        rect.y = r.y;
        SDL_RenderFillRect(Window::renderer, &rect);

        SDL_FRect pos_rect = {(float)r.x + 2, (float)r.y, (float)texture->w, (float)texture->h};
        render_texture(texture, nullptr, &pos_rect, 255);
    }
}

void render_gaintip()
{
    int h = 0;
    for (auto rit = GainTip::gaintips.rbegin(); rit != GainTip::gaintips.rend(); ++rit)
    {
        auto &[texture, destory] = *rit;
        SDL_FRect pos_rect;
        pos_rect.w = texture->w;
        pos_rect.h = texture->h;
        pos_rect.x = Camera::w - texture->w - 4;
        pos_rect.y = QuickSlot::y - h - 20;
        render_texture(texture, nullptr, &pos_rect, (float)(destory - Window::dt_now) / 5000 * 255);
        h += texture->h;
    }
}

void render_quickslot()
{
    // 渲染技能栏
    SDL_FRect pos_rect{(float)QuickSlot::x, (float)QuickSlot::y, (float)QuickSlot::w, (float)QuickSlot::h};
    render_texture(QuickSlot::backgrnd2, nullptr, &pos_rect, QuickSlot::alpha);
}

void render_uinotice()
{
    SDL_FRect pos_rect{(float)UINotice::x, (float)UINotice::y, (float)UINotice::backgrnd->w, (float)UINotice::backgrnd->h};
    render_texture(UINotice::backgrnd, nullptr, &pos_rect, UINotice::alpha);
    for (auto &[key, val] : UINotice::position_map)
    {
        auto aspr = key->second.at(key->first);
        auto position = SDL_FPoint{(float)UINotice::x + val.x + aspr.asprw->sprites[aspr.anim_index]->origin.x, (float)UINotice::y + val.y + aspr.asprw->sprites[aspr.anim_index]->origin.y};
        render_animated_sprite(position, &aspr, UINotice::alpha);
    }
}

void render_ui()
{
    for (auto it : ui_index)
    {
        switch (it)
        {
        case UIIndex::UI_UIBuff:
            render_uibuff();
            break;
        case UIIndex::UI_StatusBar:
            render_statusbar();
            break;
        case UIIndex::UI_KeyConfig:
            render_keyconfig();
            break;
        case UIIndex::UI_UIItem:
            render_uiitem();
            break;
        case UIIndex::UI_UISkill:
            render_uiskill();
            break;
        case UIIndex::UI_UIStat:
            render_uistat();
            break;
        case UIIndex::UI_WorldMap:
            render_worldmap();
            break;
        case UIIndex::UI_MiniMap:
            render_minimap();
            break;
        case UIIndex::UI_UIEquip:
            render_uiequip();
            break;
        case UIIndex::UI_QuickSlot:
            render_quickslot();
            break;
        case UIIndex::UI_UINotice:
            render_uinotice();
            break;
        default:
            break;
        }
    }
}