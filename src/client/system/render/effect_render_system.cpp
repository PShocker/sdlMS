#include "src/client/system/render/effect_render_system.h"

#include "src/client/game/game_effect.h"
#include "src/client/game/game_region_skill.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <wz/Node.h>
#include <wz/Property.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace sdlms {

void EffectRenderSystem::RenderEffect(SDL_FPoint pos, GameEffect& g_effect) {
  auto current_time = Window::dt_time_;
  if (current_time <= g_effect.delay) {
    return;
  }
  wz::Node* node;
  switch ((GameEffect::EffectSubType)g_effect.lv) {
    case GameEffect::EffectSubType::kCharacter: {
      node = WzResource::character_->find(g_effect.id);
      break;
    }
    case GameEffect::EffectSubType::kEffect: {
      node = WzResource::effect_->find(g_effect.id);
      break;
    }
    case GameEffect::EffectSubType::kItem: {
      node = WzResource::item_->find(g_effect.id);
      break;
    }
    case GameEffect::EffectSubType::kSkill: {
      node = WzResource::skill_->find(g_effect.id);
      break;
    }
  }
  auto pos2 = g_effect.pos.value();
  auto index = std::to_string(g_effect.index);
  auto texture_node = node->get_child(index);
  auto texture = WzResource::LoadTexture(texture_node);
  auto origin = WzResource::LoadFpoint(texture_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = pos.x - origin.x + pos2.x,
      .y = pos.y - origin.y + pos2.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  auto& camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
}

void EffectRenderSystem::RenderDamage(SDL_FPoint pos, GameEffect& g_effect) {
  enum class DamageType {
    kN0,
    kN1,
    kN2,
    kN3,
    kN4,
    kN5,
    kN6,
    kN7,
    kN8,
    kN9,
    kMiss,
    kGuard,
  };
  auto current_time = Window::dt_time_;
  if (current_time <= g_effect.delay) {
    return;
  }
  if (g_effect.z == false) {
    return;
  }
  static auto red = WzResource::effect_->find(u"BasicEff.img/NoRed0");
  static auto viole = WzResource::effect_->find(u"BasicEff.img/NoViolet0");
  static auto blue = WzResource::effect_->find(u"BasicEff.img/NoBlue0");

  const std::vector<std::u16string> damage_vector = {
      u"0", u"1", u"2", u"3", u"4",    u"5",
      u"6", u"7", u"8", u"9", u"Miss", u"guard",
  };
  auto& camera = CameraGameInstance::camera_;
  auto d_data = std::any_cast<DamageData>(g_effect.data);
  auto num_str = std::to_string(d_data.num);
  uint32_t w = 0;
  wz::Node* node;
  switch (d_data.type) {
    case DamageData::T::kR: {
      node = red;
      break;
    }
    case DamageData::T::kB: {
      node = blue;
      break;
    }
    case DamageData::T::kV: {
      node = viole;
      break;
    }
    default: {
      break;
    }
  }
  for (auto n : num_str) {
    auto texture_node = node->get_child(std::string{n});
    auto texture = WzResource::LoadTexture(texture_node);
    auto origin = WzResource::LoadFpoint(texture_node->get_child(u"origin"));
    auto x = pos.x - origin.x + w;
    auto y = pos.y - origin.y - g_effect.index * texture->h - 32 -
             (255 - g_effect.alpha) / 5;
    SDL_FRect pos_rect = {
        .x = x,
        .y = y,
        .w = static_cast<float>(texture->w),
        .h = static_cast<float>(texture->h),
    };
    if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
      pos_rect.x -= camera.x;
      pos_rect.y -= camera.y;
      SDL_SetTextureAlphaMod(texture, g_effect.alpha);
      SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
    }
    w += texture->w - 3;
  }
}

void EffectRenderSystem::RenderSkiUse(SDL_FPoint pos, GameEffect& g_effect,
                                      bool flip) {
  auto ski_node = SkillGameInstance::LoadSkiNode(g_effect.id);
  if (g_effect.order.has_value()) {
    ski_node =
        ski_node->get_child("effect" + std::to_string(g_effect.order.value()));
  } else {
    ski_node = ski_node->get_child(u"effect");
  }
  auto index = std::to_string(g_effect.index);
  auto texture_node = ski_node->get_child(index);
  auto texture = WzResource::LoadTexture(texture_node);
  auto origin = WzResource::LoadFpoint(texture_node->get_child(u"origin"));
  auto x = pos.x;
  auto y = pos.y;
  if (g_effect.pos.has_value()) {
    x = g_effect.pos->x;
    y = g_effect.pos->y;
    flip = g_effect.flip.value();
  }
  SDL_FRect pos_rect = {
      .x = x - origin.x,
      .y = y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  if (flip == 1) {
    pos_rect.x = x;
    pos_rect.x = (pos_rect.x - (texture->w - origin.x));
  }
  auto& camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTextureRotated(Window::renderer_, texture, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)flip);
  }
}

void EffectRenderSystem::RenderSkiAffected(SDL_FPoint pos, GameEffect& g_effect,
                                           bool flip) {
  auto ski_node = SkillGameInstance::LoadSkiNode(g_effect.id);
  if (!ski_node->get_child(u"affected")) {
    return;
  }
  ski_node = ski_node->get_child(u"affected");
  auto index = std::to_string(g_effect.index);
  auto texture_node = ski_node->get_child(index);
  auto texture = WzResource::LoadTexture(texture_node);
  auto origin = WzResource::LoadFpoint(texture_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = pos.x - origin.x,
      .y = pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  if (flip == 1) {
    pos_rect.x = pos.x;
    pos_rect.x = (pos_rect.x - (texture->w - origin.x));
  }
  auto& camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTextureRotated(Window::renderer_, texture, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)flip);
  }
}

void EffectRenderSystem::RenderSkiHit(SDL_FPoint pos, GameEffect& g_effect,
                                      bool flip) {
  auto now = Window::dt_time_;
  if (g_effect.delay >= now) {
    return;
  }
  auto ski_node = SkillGameInstance::LoadSkiNode(g_effect.id);
  if (ski_node->find(u"hit/0")) {
    ski_node = ski_node->find(u"hit/0");
  } else {
    ski_node = SkillGameInstance::LoadSkiLevelNode(g_effect.id, g_effect.lv);
    ski_node = ski_node->find(u"hit/0");
  }
  auto index = std::to_string(g_effect.index);
  auto texture_node = ski_node->get_child(index);
  auto texture = WzResource::LoadTexture(texture_node);
  if (texture_node->type == wz::Type::UOL) {
    texture_node =
        static_cast<wz::Property<wz::WzUOL>*>(texture_node)->get_uol();
  }
  auto origin = WzResource::LoadFpoint(texture_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = pos.x - origin.x + g_effect.pos->x,
      .y = pos.y - origin.y + g_effect.pos->y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  if (flip == 1) {
    pos_rect.x = pos.x;
    pos_rect.x = (pos_rect.x - (texture->w - origin.x));
  }
  auto& camera = CameraGameInstance::camera_;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_RenderTextureRotated(Window::renderer_, texture, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)flip);
  }
}

bool EffectRenderSystem::RenderCustom(SDL_FPoint pos, GameEffect& g_effect,
                                      bool flip) {
  auto skills = SkillGameInstance::Skis();
  if (skills.contains(g_effect.id)) {
    return skills.at(g_effect.id).effect(pos, &g_effect, flip);
  }
  return false;
}

void EffectRenderSystem::RenderSkiRegion(GameEffect& g_effect) {
  auto rski = std::any_cast<GameRegionSkill>(&g_effect.data);
  auto id = rski->id;
  auto ski_node = SkillGameInstance::LoadSkiNode(id);
  if (ski_node->find(u"special/x")) {
    // 311002,
    auto special = ski_node->get_child(u"special");
    auto x = static_cast<wz::Property<int>*>(special->find(u"x"))->get();
    auto y = static_cast<wz::Property<int>*>(special->find(u"y"))->get();
    auto fall = static_cast<wz::Property<int>*>(special->find(u"fall"))->get();
    auto start =
        static_cast<wz::Property<int>*>(special->find(u"start"))->get();
    auto interval =
        static_cast<wz::Property<int>*>(special->find(u"interval"))->get();
    auto count =
        static_cast<wz::Property<int>*>(special->find(u"count"))->get();
    auto duration =
        static_cast<wz::Property<int>*>(special->find(u"duration"))->get();
  } else {
  }
  return;
}

bool EffectRenderSystem::Render(SDL_FPoint pos, GameEffect& g_effect,
                                bool flip) {
  bool r = true;
  switch (g_effect.type) {
    case GameEffect::EffectType::kEffect: {
      RenderEffect(pos, g_effect);
      break;
    }
    case GameEffect::EffectType::kDamage: {
      RenderDamage(pos, g_effect);
      break;
    }
    case GameEffect::EffectType::kSkillUse: {
      RenderSkiUse(pos, g_effect, flip);
      break;
    }
    case GameEffect::EffectType::kSkillAffected: {
      RenderSkiAffected(pos, g_effect, flip);
      break;
    }
    case GameEffect::EffectType::kSkillHit: {
      RenderSkiHit(pos, g_effect, flip);
      break;
    }
    case GameEffect::EffectType::kSkillCustom: {
      r = RenderCustom(pos, g_effect, flip);
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

bool EffectRenderSystem::RenderMobEffects(GameMob& g_mob, bool front) {
  auto& v = g_mob.effect;
  for (auto& e : v) {
    if (e.z.has_value() && e.z.value() == front) {
      Render(g_mob.pos, e, g_mob.flip);
    }
  }
  return true;
}

bool EffectRenderSystem::RenderCharacterEffects(GameCharacter* g_character,
                                                bool front) {
  auto& effect = g_character->effect;
  for (auto it = effect.begin(); it != effect.end();) {
    auto& e = *it;
    if (e.z.has_value() && e.z.value() == front) {
      auto r = Render(g_character->pos, e, g_character->flip);
      if (r == false) {
        it = effect.erase(it);
        continue;
      }
    }
    ++it;
  }
  return true;
}

bool EffectRenderSystem::Render(GameEffect& g_effect) {
  const auto& pos = g_effect.pos.value();
  const auto& flip = g_effect.flip.value();
  EffectRenderSystem::Render(pos, g_effect, flip);
  return true;
}

}  // namespace sdlms
