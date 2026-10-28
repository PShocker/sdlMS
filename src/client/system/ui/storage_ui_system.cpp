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

SDL_FPoint StorageUiSystem::load_wh() { return {421, 366}; }

bool StorageUiSystem::cursor_in() { return false; }

std::optional<uint32_t> StorageUiSystem::load_mouse_index() {
  auto cursor_in = CursorGameInstance::cursor_ui;
  if (cursor_in != render) {
    return std::nullopt;
  }
  SDL_FPoint slot_pos{8, 51};
  const auto slot_space_x = 4;
  const auto slot_space_y = 2;

  auto &mouse_pos = Window::mouse_pos;
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

void StorageUiSystem::render_backgrnd() {
  static auto back_texture = WzResource::load_texture(
      WzResource::ui->find(u"Storage.img/Trunk/backgrnd"));
  SDL_FRect pos_rect = {
      pos.x,
      pos.y,
      (float)back_texture->w,
      (float)back_texture->h,
  };
  SDL_RenderTexture(Window::renderer, back_texture, nullptr, &pos_rect);
}

void StorageUiSystem::render_items() {
  auto &items0 = StorageGameInstance::storage.data;
  auto &items1 = PackageGameInstance::data[active_tab];
  int m = 0;
  for (auto items : {items0, items1}) {
    for (uint8_t i = 0; i <= items.size(); i++) {
      auto row = i / 5;
      auto col = i % 5;
    }
    m++;
  }
}

void StorageUiSystem::render_vscr() {}

void StorageUiSystem::render_button() {
  const static std::array buttons_node = {
      WzResource::ui->find(u"Storage.img/Trunk/button:get"),
      WzResource::ui->find(u"Storage.img/Trunk/button:put"),
      WzResource::ui->find(u"Storage.img/Trunk/button:sort"),
      WzResource::ui->find(u"Storage.img/Trunk/button:exit"),
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
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        CursorGameInstance::modal_overlay == render) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void StorageUiSystem::render_tab() {
  auto t = WzResource::load_texture(
      WzResource::ui->find(u"Storage.img/Trunk/tab:get/selected/0"));
  SDL_FRect pos_rect{
      static_cast<float>((int)pos.x + 6),
      static_cast<float>((int)pos.y + 98),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
}

bool StorageUiSystem::render() { return true; }

void StorageUiSystem::toggle() {
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void StorageUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void StorageUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
}

bool StorageUiSystem::event_button(SDL_Event *event) {
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
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

void StorageUiSystem::event_tab() {
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
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      active_tab = i;
    }
  }
  return;
}

void StorageUiSystem::event_close() { close(); }

void StorageUiSystem::event_button_get() {
  if (active_item[0].has_value()) {
    auto &itm = StorageGameInstance::storage.data.at(active_item[0].value());
    if (itm->id.empty()) {
      return;
    }
    if (PackageGameInstance::add_item(itm)) {
      itm->id = u"";
    } else {
      NoticeUiSystem::open_no_space(itm->type);
    }
  }
  return;
}

bool StorageUiSystem::add_item(std::polymorphic<GameItem> &item) {
  if (item->id.empty()) {
    return true;
  }
  for (auto &itm : StorageGameInstance::storage.data) {
    if (itm->id.empty()) {
      itm = item;
      return true;
    }
  }
  return false;
}

void StorageUiSystem::event_button_put() {
  if (active_item[1].has_value()) {
    auto &itm =
        PackageGameInstance::data[active_tab].at(active_item[1].value());
    if (!add_item(itm)) {
      NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kNoStorageSpace;
      NoticeUiSystem::open();
    }
  }
  return;
}

void StorageUiSystem::event_button_sort() {
  const auto cmp = [](const std::polymorphic<GameItem> &a,
                      const std::polymorphic<GameItem> &b) {
    bool aEmpty = a->id.empty();
    bool bEmpty = b->id.empty();
    if (aEmpty != bEmpty)
      return bEmpty;
    return a->id < b->id;
  };
  std::ranges::sort(StorageGameInstance::storage.data, cmp);
}

void StorageUiSystem::event_button_exit() { event_close(); }

bool StorageUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
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