#include "equip_ui_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>

void equip_ui_system::render_backgrnd() {
  static auto back_node = wz_resource::ui->find(u"Equipment.img/equip");
  static auto texture =
      wz_resource::load_texture(back_node->get_child(u"backgrnd"));

  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

void equip_ui_system::render_equip() {}

bool equip_ui_system::render() {
  render_backgrnd();
  render_equip();
  return true;
}

void equip_ui_system::open() {
  system::render_systems.push_back(render);
  system::event_systems.push_back(event);
}

void equip_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void equip_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool equip_ui_system::cursor_in() { return true; }

bool equip_ui_system::event(SDL_Event *event) { return true; }