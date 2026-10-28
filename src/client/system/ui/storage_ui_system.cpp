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

SDL_FPoint StorageUiSystem::LoadWh() { return {421, 366}; }

bool StorageUiSystem::CursorIn() { return false; }

std::optional<uint32_t> StorageUiSystem::LoadMouseIndex() {
  auto CursorIn = CursorGameInstance::cursor_ui;
  if (CursorIn != Render) {
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

void StorageUiSystem::RenderBackgrnd() {
  static auto back_texture = WzResource::LoadTexture(
      WzResource::ui->find(u"Storage.img/Trunk/backgrnd"));
  SDL_FRect pos_rect = {
      pos.x,
      pos.y,
      (float)back_texture->w,
      (float)back_texture->h,
  };
  SDL_RenderTexture(Window::renderer, back_texture, nullptr, &pos_rect);
}

void StorageUiSystem::RenderItems() {
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

void StorageUiSystem::RenderVscr() {}

void StorageUiSystem::RenderButton() {
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
    auto CursorIn = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && CursorIn == Render &&
        CursorGameInstance::modal_overlay == Render) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::LoadTexture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::LoadTexture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::LoadTexture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void StorageUiSystem::RenderTab() {
  auto t = WzResource::LoadTexture(
      WzResource::ui->find(u"Storage.img/Trunk/tab:get/selected/0"));
  SDL_FRect pos_rect{
      static_cast<float>((int)pos.x + 6),
      static_cast<float>((int)pos.y + 98),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
}

bool StorageUiSystem::Render() { return true; }

void StorageUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems, fn)) {
    Close();
  } else {
    Open();
  }
}

void StorageUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::Render);
  if (it != System::render_systems.end()) {
    auto wh = LoadWh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    System::render_systems.insert(it, Render);
    System::event_systems.insert(System::event_systems.begin(), Event);
  }
}

void StorageUiSystem::Close() {
  std::erase(System::render_systems, Render);
  std::erase(System::event_systems, Event);
}

bool StorageUiSystem::EventButton(SDL_Event *Event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  r = {
      SDL_FRect{132, 31, 70, 18}, //
      SDL_FRect{340, 11, 70, 18}, //
      SDL_FRect{132, 51, 70, 19}, //
      SDL_FRect{132, 11, 70, 18}, //
  };
  fns = {
      EventButtonGet,
      EventButtonPut,
      EventButtonSort,
      EventButtonExit,
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

void StorageUiSystem::EventTab() {
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

void StorageUiSystem::EventClose() { Close(); }

void StorageUiSystem::EventButtonGet() {
  if (active_item[0].has_value()) {
    auto &itm = StorageGameInstance::storage.data.at(active_item[0].value());
    if (itm->id.empty()) {
      return;
    }
    if (PackageGameInstance::AddItem(itm)) {
      itm->id = u"";
    } else {
      NoticeUiSystem::OpenNoSpace(itm->type);
    }
  }
  return;
}

bool StorageUiSystem::AddItem(std::polymorphic<GameItem> &item) {
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

void StorageUiSystem::EventButtonPut() {
  if (active_item[1].has_value()) {
    auto &itm =
        PackageGameInstance::data[active_tab].at(active_item[1].value());
    if (!AddItem(itm)) {
      NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kNoStorageSpace;
      NoticeUiSystem::Open();
    }
  }
  return;
}

void StorageUiSystem::EventButtonSort() {
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

void StorageUiSystem::EventButtonExit() { EventClose(); }

bool StorageUiSystem::Event(SDL_Event *Event) {
  bool r = true;
  switch (Event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        EventTab();
        r = !EventButton(Event);
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