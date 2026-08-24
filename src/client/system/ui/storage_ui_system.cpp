#include "storage_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/storage_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <cstdint>
#include <optional>

SDL_FPoint storage_ui_system::load_wh() { return {421, 366}; }

bool storage_ui_system::cursor_in() { return false; }

std::optional<uint32_t> storage_ui_system::load_mouse_index() {
  auto cursor_in = cursor_game_instance::cursor_ui;
  if (cursor_in != render) {
    return std::nullopt;
  }
  SDL_FPoint slot_pos{8, 51};
  const auto slot_space_x = 4;
  const auto slot_space_y = 2;

  auto &mouse_pos = window::mouse_pos;
  auto lx = pos.x + slot_pos.x;
  auto rx = lx + 36 * 5;

  auto ty = pos.y + slot_pos.y;
  auto by = ty + 34 * 6;
  if ((lx <= mouse_pos.x && mouse_pos.x <= rx) &&
      (ty <= mouse_pos.y && mouse_pos.y <= by)) {
    auto dx = int((mouse_pos.x - lx) / 36);
    auto dy = int((mouse_pos.y - ty) / 34);
    auto index = dy * 5 + dx;
    return index;
  }

  return std::nullopt;
}

void storage_ui_system::render_backgrnd() {
  static auto back_texture = wz_resource::load_texture(
      wz_resource::ui->find(u"Storage.img/Trunk/backgrnd"));
  SDL_FRect pos_rect = {
      pos.x,
      pos.y,
      (float)back_texture->w,
      (float)back_texture->h,
  };
  SDL_RenderTexture(window::renderer, back_texture, nullptr, &pos_rect);
}

void storage_ui_system::render_items() {
  auto &items0 = storage_game_instance::storage.data;
  auto &items1 = package_game_instance::data[active_tab];
  int m = 0;
  for (auto items : {items0, items1}) {
    for (uint8_t i = 0; i <= items.size(); i++) {
      auto row = i / 5;
      auto col = i % 5;
    }
    m++;
  }
}

void storage_ui_system::render_vscr() {
  
}

void storage_ui_system::render_button() {
  const static std::array buttons_node = {
      wz_resource::ui->find(u"Storage.img/Trunk/button:get"),
      wz_resource::ui->find(u"Storage.img/Trunk/button:put"),
      wz_resource::ui->find(u"Storage.img/Trunk/button:sort"),
      wz_resource::ui->find(u"Storage.img/Trunk/button:exit"),
  };
  const std::array buttons_rect = {
      SDL_FRect{132, 31, 70, 18}, //
      SDL_FRect{340, 11, 70, 18}, //
      SDL_FRect{132, 51, 70, 19}, //
      SDL_FRect{132, 11, 70, 18}, //
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    auto &mouse_pos = window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        cursor_game_instance::modal_overlay == render) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = wz_resource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void storage_ui_system::render_tab() {
  auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Storage.img/Trunk/tab:get/selected/0"));
  SDL_FRect pos_rect{
      static_cast<float>((int)pos.x + 6),
      static_cast<float>((int)pos.y + 98),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
}

bool storage_ui_system::render() { return true; }

void storage_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void storage_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void storage_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

bool storage_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  r = {
      SDL_FRect{132, 31, 70, 18}, //
      SDL_FRect{340, 11, 70, 18}, //
      SDL_FRect{132, 51, 70, 19}, //
      SDL_FRect{132, 11, 70, 18}, //
  };
  fns = {
      event_button_get,
      event_button_put,
      event_button_sort,
      event_button_exit,
  };

  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

void storage_ui_system::event_tab() {
  const static std::array tab_rect = {
      SDL_FRect{5, 24, 33, 19},   //
      SDL_FRect{38, 24, 33, 19},  //
      SDL_FRect{71, 24, 33, 19},  //
      SDL_FRect{104, 24, 33, 19}, //
      SDL_FRect{137, 24, 33, 19}, //
      SDL_FRect{170, 24, 33, 19}, //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      active_tab = i;
    }
  }
  return;
}

void storage_ui_system::event_close() { close(); }

void storage_ui_system::event_button_get() {
  if (active_item[0].has_value()) {
    auto &itm = storage_game_instance::storage.data.at(active_item[0].value());
    if (itm->id.empty()) {
      return;
    }
    if (package_game_instance::add_item(itm)) {
      itm->id = u"";
    } else {
      notice_ui_system::type = notice_ui_system::notice_enum::shopbuy_no_space;
      notice_ui_system::open();
    }
  }
  return;
}

bool storage_ui_system::add_item(std::polymorphic<game_item> &item) {
  if (item->id.empty()) {
    return true;
  }
  for (auto &itm : storage_game_instance::storage.data) {
    if (itm->id.empty()) {
      itm = item;
      return true;
    }
  }
  return false;
}

void storage_ui_system::event_button_put() {
  if (active_item[1].has_value()) {
    auto &itm =
        package_game_instance::data[active_tab].at(active_item[1].value());
    if (!add_item(itm)) {
      notice_ui_system::type = notice_ui_system::notice_enum::shopbuy_no_space;
      notice_ui_system::open();
    }
  }
  return;
}

void storage_ui_system::event_button_sort() {
  const auto cmp = [](const std::polymorphic<game_item> &a,
                      const std::polymorphic<game_item> &b) {
    bool aEmpty = a->id.empty();
    bool bEmpty = b->id.empty();
    if (aEmpty != bEmpty)
      return bEmpty;
    return a->id < b->id;
  };
  std::ranges::sort(storage_game_instance::storage.data, cmp);
}

void storage_ui_system::event_button_exit() { event_close(); }

bool storage_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        event_tab();
        r = !event_button(event);
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    break;
  }
  default: {
    break;
  }
  }

  return r;
}