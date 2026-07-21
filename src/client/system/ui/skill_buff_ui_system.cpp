#include "skill_buff_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_skill.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "tooltip_ui_system.h"
#include <cstddef>
#include <cstdint>
#include <optional>

static std::optional<game_skill> mouse_ski;

void skill_buff_ui_system::render_ui(game_skill &sk, float x, float y) {
  auto id = sk.id;
  auto ski_node = skill_game_instance::load_ski_node(id);
  auto icon = wz_resource::load_texture(ski_node->get_child(u"icon"));
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(window::renderer, icon, NULL, &pos_rect);
  auto &mouse_pos = window::mouse_pos;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    mouse_ski = sk;
  }
}

void skill_buff_ui_system::render_ui_info() {
  if (!mouse_ski.has_value()) {
    return;
  }
  auto ski = mouse_ski.value();
  // 宽度固定330
  const auto w = 330;
  auto &mouse_pos = window::mouse_pos;
  const auto &camera = camera_game_instance::camera;
  auto x = 0;
  if (mouse_pos.x + w <= camera.w) {
    x = mouse_pos.x;
  } else {
    x = camera.w - 330;
  }
  tooltip_ui_system::render_skill(ski.id, ski.lv, x, mouse_pos.y);
}

bool skill_buff_ui_system::render() {
  mouse_ski = std::nullopt;
  // ski buff
  const auto &camera = camera_game_instance::camera;
  for (uint32_t i = 0; i < skill_game_instance::ski.size(); i++) {
    auto col = i % 5;
    auto row = i / 5;
    auto x = camera.w - 32 - col * 32;
    auto y = row * 32;
    render_ui(skill_game_instance::ski[i], x, y);
  }
  render_ui_info();
  return true;
}

bool skill_buff_ui_system::event(SDL_Event *event) { return true; }