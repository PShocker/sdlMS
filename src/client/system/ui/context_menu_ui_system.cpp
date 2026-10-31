#include "src/client/system/ui/context_menu_ui_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_info_ui_system.h"
#include "src/client/system/ui/component/ui_component.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_main.h"

#include <SDL3/SDL_render.h>

#include <cstddef>

namespace sdlms {

SDL_FPoint ContextMenuUiSystem::LoadWh() {
  return {100, 122};
}

bool ContextMenuUiSystem::CursorIn() {
  bool r = false;
  auto& mouse = Window::mouse_pos_;
  auto [w, h] = LoadWh();
  SDL_FRect pos_rect{pos_.x + 3, pos_.y, w - 3, h};
  r = SDL_PointInRectFloat(&mouse, &pos_rect);
  return r;
}

void ContextMenuUiSystem::RenderBackgrnd() {
  auto [w, h] = LoadWh();
  static auto t2 = WzResource::LoadTexture(
      WzResource::ui_->find(u"ContextMenu.img/ContextMenu/t2"));
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(t2->w),
      static_cast<float>(t2->h),
  };
  SDL_RenderTexture(Window::renderer_, t2, nullptr, &pos_rect);
  static auto c = WzResource::LoadTexture(
      WzResource::ui_->find(u"ContextMenu.img/ContextMenu/c"));
  static auto s = WzResource::LoadTexture(
      WzResource::ui_->find(u"ContextMenu.img/ContextMenu/s"));
  pos_rect = {
      pos_.x + 3,
      pos_.y + t2->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t2->h - s->h),
  };
  SDL_RenderTextureTiled(Window::renderer_, c, nullptr, 1, &pos_rect);

  pos_rect = {
      pos_.x + 3,
      pos_.y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_RenderTexture(Window::renderer_, s, nullptr, &pos_rect);
}


UIComponents& ContextMenuUiSystem::GetComponent() {
  const static std::array<Button::Callback, 7> kCallbacks = {
      EventButtonInfo,  EventButtonSave,   EventButtonWhisper, EventButtonParty,
      EventButtonTrade, EventButtonFriend, EventButtonChat,
  };

  components_.clear();
  const static std::array buttons_nodes = {
      WzResource::ui_->find(u"ContextMenu.img/ContextMenu/BtInfo"),
      WzResource::ui_->find(u"ContextMenu.img/ContextMenu/BtSave"),
      WzResource::ui_->find(u"ContextMenu.img/ContextMenu/BtWhisper"),
      WzResource::ui_->find(u"ContextMenu.img/ContextMenu/BtParty"),
      WzResource::ui_->find(u"ContextMenu.img/ContextMenu/BtTrade"),
      WzResource::ui_->find(u"ContextMenu.img/ContextMenu/BtFriend"),
      WzResource::ui_->find(u"ContextMenu.img/ContextMenu/BtMapleChat"),
  };
  std::array buttons_rect = {
      SDL_FRect{pos_.x + 9, pos_.y + 6, 85, 14},
      SDL_FRect{pos_.x + 9, pos_.y + 22, 85, 14},
      SDL_FRect{pos_.x + 9, pos_.y + 38, 85, 14},
      SDL_FRect{pos_.x + 9, pos_.y + 54, 85, 14},
      SDL_FRect{pos_.x + 9, pos_.y + 70, 85, 14},
      SDL_FRect{pos_.x + 9, pos_.y + 86, 85, 14},
      SDL_FRect{pos_.x + 9, pos_.y + 102, 85, 14},
  };
  std::vector<bool> disable = {
      false, false, false, false, false, false, false,
  };
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x = (int)rect.x;
    rect.y = (int)rect.y;
    Button button{.node = buttons_nodes[i],
                  .rect = rect,
                  .enable = !disable[i],
                  .focused = true};
    components_.push_back(button);
    if (i < kCallbacks.size()) {
      std::get<Button>(components_.back()).callback = kCallbacks[i];
    }
  }
  return components_;
}


bool ContextMenuUiSystem::Render() {
  RenderBackgrnd();
  RenderUI();
  return true;
}

void ContextMenuUiSystem::EventButtonInfo() {
  if (client_id_ == ServerMain::local_addr_) {
    // self
    CharacterInfoUiSystem::character_ = CharacterGameInstance::self_;
    CharacterInfoUiSystem::Close();
    CharacterInfoUiSystem::Open();
  } else if (CharacterGameInstance::others_.contains(client_id_)) {
    CharacterInfoUiSystem::character_ =
        CharacterGameInstance::others_[client_id_].g_character;
    CharacterInfoUiSystem::Close();
    CharacterInfoUiSystem::Open();
  } else {
    fbs::ClientCharacterInfoT ct;
    ct.payload = client_id_;
    ClientRequest::SendToHost(ct);
  }
}

void ContextMenuUiSystem::EventButtonSave() {
  auto name = client_name_;
  auto utf8_str = TextGameInstance::LoadU8str(name);
  SDL_SetClipboardText(utf8_str.c_str());
}

void ContextMenuUiSystem::EventButtonWhisper() {}

void ContextMenuUiSystem::EventButtonParty() {}

void ContextMenuUiSystem::EventButtonTrade() {
  fbs::ClientCharacterTradeT ct;
  ct.to_id = client_id_;
  ClientRequest::SendToHost(ct);
}

void ContextMenuUiSystem::EventButtonFriend() {}

void ContextMenuUiSystem::EventButtonChat() {}

bool ContextMenuUiSystem::EventUI(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), true);
}


void ContextMenuUiSystem::EventClose() {
  Close();
};

bool ContextMenuUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = event->key.scancode;
      switch (scan_code) {
        case SDL_SCANCODE_ESCAPE: {
          EventClose();
          return false;
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
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
          EventUI(event);
        }
      }
      EventClose();
      r = false;
      break;
    }
    default: {
      break;
    }
  }

  return r;
}
void ContextMenuUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void ContextMenuUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

}  // namespace sdlms
