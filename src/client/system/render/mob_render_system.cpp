#include "mob_render_system.h"
#include "gauge_render_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system/render/effect_render_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <algorithm>
#include <string>

bool mob_render_system::render_mob(game_mob &g_mob) {
  auto action_type = mob_logic_system::load_action_type(g_mob.action);
  if (action_type == mob_logic_system::action_enum::revive) {
    return false;
  }
  auto mob_node = mob_game_instance::load_link_mob_node(g_mob.id);

  auto action_node = mob_node->get_child(g_mob.action);

  bool zigzag = action_node->get_child(u"zigzag") == nullptr ? false : true;
  int32_t canvas_count = action_node->children_count();
  if (zigzag) {
    canvas_count -= 1;
  }
  std::string frame_index;
  if (zigzag && g_mob.ani_index >= canvas_count) {
    uint32_t period = 2 * (canvas_count - 1);
    uint32_t phase = g_mob.ani_index % period;
    uint32_t frame_i = (phase <= canvas_count - 1) ? phase : (period - phase);
    frame_index = std::to_string(frame_i);
  } else {
    frame_index = std::to_string(g_mob.ani_index);
  }

  mob_node = action_node->get_child(frame_index);
  if (mob_node->type == wz::Type::UOL) {
    mob_node = static_cast<wz::Property<wz::WzUOL> *>(mob_node)->get_uol();
  }

  auto texture = wz_resource::load_texture(mob_node);

  auto v =
      static_cast<wz::Property<wz::WzVec2D> *>(mob_node->get_child(u"origin"))
          ->get();
  auto origin = SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)};

  SDL_FRect pos_rect = {
      .x = g_mob.pos.x - origin.x,
      .y = g_mob.pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  if (g_mob.flip == 1) {
    pos_rect.x = g_mob.pos.x;
    pos_rect.x = (pos_rect.x - (texture->w - origin.x));
  }
  auto &camera = camera_game_instance::camera;
  if (SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    pos_rect.x -= camera.x;
    pos_rect.y -= camera.y;
    SDL_SetTextureAlphaMod(texture, g_mob.alpha);
    SDL_RenderTextureRotated(window::renderer, texture, nullptr, &pos_rect, 0,
                             nullptr, (SDL_FlipMode)g_mob.flip);
  }

  return true;
}

bool mob_render_system::render_gauge(game_mob &g_mob) {
  if (g_mob.gauge.has_value()) {
    auto r = mob_logic_system::load_rect(g_mob);
    if (!r.has_value()) {
      return false;
    }
    auto pos = g_mob.pos;
    auto head = mob_logic_system::load_head(g_mob);
    pos.y = pos.y + head->y - 20;
    auto hp_percent_now = (float)g_mob.hp / g_mob.max_hp;
    hp_percent_now = std::max(0.0f, hp_percent_now);
    g_mob.gauge->hp_percent =
        std::lerp(g_mob.gauge->hp_percent, hp_percent_now, 0.03);
    g_mob.gauge->hp_percent_now = hp_percent_now;
    gauge_render_system::render(pos, g_mob.gauge.value());
    return true;
  }
  return false;
}

bool mob_render_system::render_effect_back(game_mob &g_mob) {
  return effect_render_system::render_mob_back(g_mob);
}

bool mob_render_system::render(game_mob &g_mob) {
  render_mob(g_mob);
  render_effect_back(g_mob);
  render_gauge(g_mob);
  render_nametag(g_mob);
  // auto r = mob_logic_system::load_rect(g_mob);
  // if (r.has_value()) {
  //   SDL_FRect rr = r.value();
  //   auto &camera = camera_game_instance::camera;
  //   rr.x -= camera.x;
  //   rr.y -= camera.y;
  //   SDL_RenderFillRect(window::renderer, &rr);
  // }

  return true;
}

void mob_render_system::render_nametag(const std::u16string &text,
                                       SDL_FPoint p) {
  const auto &camera = camera_game_instance::camera;
  freetype::load_aligned(true);
  freetype::load_size(13);
  auto w = freetype::load_w(text);
  auto h = freetype::load_lh();

  auto x = p.x;
  auto y = p.y;
  SDL_FRect rect;
  rect.w = w + 4;
  rect.h = h + 4;
  rect.x = x - rect.w / 2;
  rect.y = y;
  if (SDL_HasRectIntersectionFloat(&rect, &camera)) {
    rect.x -= camera.x;
    rect.y -= camera.y;
    SDL_SetRenderDrawBlendMode(window::renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(window::renderer, 0, 0, 0, 178);
    SDL_RenderFillRect(window::renderer, &rect);
    freetype::load_color(255, 255, 255, 255);
    freetype::draw_line(text, x - camera.x - w / 2, y - camera.y);
  }
  return;
}

bool mob_render_system::render_nametag(game_mob &g_mob) {
  if (!g_mob.gauge.has_value()) {
    return false;
  }
  auto r = mob_logic_system::load_rect(g_mob);
  if (!r.has_value()) {
    return false;
  }
  auto mob_name = mob_game_instance::load_mob_name(g_mob.id);
  render_nametag(mob_name, {g_mob.pos.x, g_mob.pos.y + 5});
  return true;
}