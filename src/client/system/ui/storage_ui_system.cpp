#include "src/client/system/ui/storage_ui_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/storage_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/button.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <algorithm>
#include <cstdint>
#include <optional>

namespace sdlms {

SDL_FPoint StorageUiSystem::LoadWh() {
  return {421, 366};
}

bool StorageUiSystem::CursorIn() {
  return false;
}

std::optional<uint32_t> StorageUiSystem::LoadMouseIndex() {
  auto cursor_in = CursorGameInstance::cursor_ui_;
  if (cursor_in != Render) {
    return std::nullopt;
  }
  SDL_FPoint slot_pos{8, 51};
  const auto slot_space_x = 4;
  const auto slot_space_y = 2;

  auto& mouse_pos = Window::mouse_pos_;
  auto lx = pos_.x + slot_pos.x;
  auto rx = lx + 36 * 5;

  auto ty = pos_.y + slot_pos.y;
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
      WzResource::ui_->find(u"Storage.img/Trunk/backgrnd"));
  SDL_FRect pos_rect = {
      pos_.x,
      pos_.y,
      (float)back_texture->w,
      (float)back_texture->h,
  };
  SDL_RenderTexture(Window::renderer_, back_texture, nullptr, &pos_rect);
}

void StorageUiSystem::RenderItems() {
  auto& items0 = StorageGameInstance::storage_.data;
  auto& items1 = PackageGameInstance::data_[active_tab_];
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
      WzResource::ui_->find(u"Storage.img/Trunk/button:get"),
      WzResource::ui_->find(u"Storage.img/Trunk/button:put"),
      WzResource::ui_->find(u"Storage.img/Trunk/button:sort"),
      WzResource::ui_->find(u"Storage.img/Trunk/button:exit"),
  };
  const std::array buttons_rect = {
      SDL_FRect{132, 31, 70, 18},  //
      SDL_FRect{340, 11, 70, 18},  //
      SDL_FRect{132, 51, 70, 19},  //
      SDL_FRect{132, 11, 70, 18},  //
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x += (int)pos_.x;
    rect.y += (int)pos_.y;
    Button{.node = buttons_node[i],
           .rect = rect,
           .enable = true,
           .focused = CursorGameInstance::cursor_ui_ == Render &&
                      !CursorGameInstance::modal_overlay_}
        .Render();
  }
}

void StorageUiSystem::RenderTab() {
  auto t = WzResource::LoadTexture(
      WzResource::ui_->find(u"Storage.img/Trunk/tab:get/selected/0"));
  SDL_FRect pos_rect{
      static_cast<float>((int)pos_.x + 6),
      static_cast<float>((int)pos_.y + 98),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
}

bool StorageUiSystem::Render() {
  return true;
}

void StorageUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

void StorageUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void StorageUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

bool StorageUiSystem::EventButton(SDL_Event* event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  r = {
      SDL_FRect{132, 31, 70, 18},  //
      SDL_FRect{340, 11, 70, 18},  //
      SDL_FRect{132, 51, 70, 19},  //
      SDL_FRect{132, 11, 70, 18},  //
  };
  fns = {
      EventButtonGet,
      EventButtonPut,
      EventButtonSort,
      EventButtonExit,
  };

  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

void StorageUiSystem::EventTab() {
  const static std::array tab_rect = {
      SDL_FRect{5, 24, 33, 19},    //
      SDL_FRect{38, 24, 33, 19},   //
      SDL_FRect{71, 24, 33, 19},   //
      SDL_FRect{104, 24, 33, 19},  //
      SDL_FRect{137, 24, 33, 19},  //
      SDL_FRect{170, 24, 33, 19},  //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      active_tab_ = i;
    }
  }
  return;
}

void StorageUiSystem::EventClose() {
  Close();
}

void StorageUiSystem::EventButtonGet() {
  if (active_item_[0].has_value()) {
    auto& itm = StorageGameInstance::storage_.data.at(active_item_[0].value());
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

bool StorageUiSystem::AddItem(std::polymorphic<GameItem>& item) {
  if (item->id.empty()) {
    return true;
  }
  for (auto& itm : StorageGameInstance::storage_.data) {
    if (itm->id.empty()) {
      itm = item;
      return true;
    }
  }
  return false;
}

void StorageUiSystem::EventButtonPut() {
  if (active_item_[1].has_value()) {
    auto& itm =
        PackageGameInstance::data_[active_tab_].at(active_item_[1].value());
    if (!AddItem(itm)) {
      NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kNoStorageSpace;
      NoticeUiSystem::Open();
    }
  }
  return;
}

void StorageUiSystem::EventButtonSort() {
  const auto cmp = [](const std::polymorphic<GameItem>& a,
                      const std::polymorphic<GameItem>& b) {
    bool aEmpty = a->id.empty();
    bool bEmpty = b->id.empty();
    if (aEmpty != bEmpty)
      return bEmpty;
    return a->id < b->id;
  };
  std::ranges::sort(StorageGameInstance::storage_.data, cmp);
}

void StorageUiSystem::EventButtonExit() {
  EventClose();
}

bool StorageUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          EventTab();
          r = !EventButton(event);
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

}  // namespace sdlms
