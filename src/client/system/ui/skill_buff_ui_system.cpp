#include "skill_buff_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <cstddef>
#include <cstdint>

static void render_ui(game_skill &sk, float x, float y) {
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
}

bool skill_buff_ui_system::render() {
  // ski buff
  const auto &camera = camera_game_instance::camera;
  for (uint32_t i = 0; i < skill_game_instance::ski.size(); i++) {
    auto col = i % 5;
    auto row = i / 5;
    auto x = camera.w - 32 - col * 32;
    auto y = camera.h - 32 - row * 32;
    render_ui(skill_game_instance::ski[i], x, y);
  }
  return true;
}

bool skill_buff_ui_system::event(SDL_Event *event) { return true; }