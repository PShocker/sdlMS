#include "src/client/system/ui/login_notice_ui_system.h"

#include "src/client/component/button.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>

#include <string>
#include <vector>

namespace sdlms {

SDL_FPoint LoginNoticeUiSystem::LoadPos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto& camera = CameraGameInstance::camera_;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void LoginNoticeUiSystem::RenderBackgrnd() {
  const auto w = 1366;
  const auto h = 768;
  SDL_Texture* backgrnd;
  SDL_Texture* text;
  SDL_FPoint text_pos;
  static auto notice_node = WzResource::ui_->find(u"Login.img/Notice");
  auto [bx, by] = LoadPos();
  switch (type_) {
    case LoginNoticeSystemInstance::NoticeType::kCharacternameError: {
      backgrnd = WzResource::LoadTexture(notice_node->find(u"backgrnd/0"));
      text =
          WzResource::LoadTexture(notice_node->find(u"text/cannotUseThisName"));
      text_pos = {17, 13};
      break;
    }
    case LoginNoticeSystemInstance::NoticeType::kCharacternameUsed: {
      backgrnd = WzResource::LoadTexture(notice_node->find(u"backgrnd/0"));
      text = WzResource::LoadTexture(notice_node->find(u"text/8"));
      text_pos = {17, 13};
      break;
    }
    case LoginNoticeSystemInstance::NoticeType::kCharacterDelete: {
      backgrnd = WzResource::LoadTexture(notice_node->find(u"backgrnd/0"));
      text = WzResource::LoadTexture(notice_node->find(u"text/58"));
      text_pos = {17, 13};
      break;
    }
    case LoginNoticeSystemInstance::NoticeType::kCharacterFull: {
      backgrnd = WzResource::LoadTexture(notice_node->find(u"backgrnd/0"));
      text = WzResource::LoadTexture(
          notice_node->find(u"text/cannotCreateAccountMore"));
      text_pos = {17, 13};
      break;
    }
    case LoginNoticeSystemInstance::NoticeType::kCharacterUseAp: {
      backgrnd = WzResource::LoadTexture(notice_node->find(u"backgrnd/0"));
      text = WzResource::LoadTexture(notice_node->find(u"text/useAllAP"));
      text_pos = {17, 13};
      break;
    }
    case LoginNoticeSystemInstance::NoticeType::kLogining: {
      backgrnd =
          WzResource::LoadTexture(notice_node->find(u"Loading/backgrnd"));
      text = nullptr;
      break;
    }
    case LoginNoticeSystemInstance::NoticeType::kLoginingUsed: {
      backgrnd =
          WzResource::LoadTexture(notice_node->find(u"Loading/backgrnd"));
      text = WzResource::LoadTexture(notice_node->find(u"text/8"));
      text_pos = {17, 13};
      break;
    }
  }
  SDL_FRect pos_rect{
      bx + (w - backgrnd->w) / 2,
      by + (h - backgrnd->h) / 2,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);

  if (text != nullptr) {
    pos_rect.x += text_pos.x;
    pos_rect.y += text_pos.y;
    pos_rect.w = text->w;
    pos_rect.h = text->h;
    SDL_RenderTexture(Window::renderer_, text, nullptr, &pos_rect);
  }
  switch (type_) {
    case LoginNoticeSystemInstance::NoticeType::kLogining: {
      static auto circle_node =
          WzResource::ui_->find(u"Login.img/Notice/Loading/circle");
      const auto circle_delay = 90;
      auto now = Window::dt_now_;
      auto index = (now / circle_delay) % (circle_node->children_count());
      auto circle_texture = circle_node->get_child(std::to_string(index));
      auto circle = WzResource::LoadTexture(circle_texture);
      pos_rect.x += 84;
      pos_rect.y += 40;
      pos_rect.w = circle->w;
      pos_rect.h = circle->h;
      SDL_RenderTexture(Window::renderer_, circle, nullptr, &pos_rect);
      break;
    }
    default: {
      break;
    }
  }
}

void LoginNoticeUiSystem::RenderButton() {
  components_.clear();
  const auto w = 1366;
  const auto h = 768;
  std::vector<wz::Node*> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  auto [bx, by] = LoadPos();
  switch (type_) {
    case LoginNoticeSystemInstance::NoticeType::kCharacternameError:
    case LoginNoticeSystemInstance::NoticeType::kCharacterFull:
    case LoginNoticeSystemInstance::NoticeType::kCharacterUseAp:
    case LoginNoticeSystemInstance::NoticeType::kLoginingUsed:
    case LoginNoticeSystemInstance::NoticeType::kCharacternameUsed: {
      buttons_nodes = {
          WzResource::ui_->find(u"Login.img/Notice/BtYes"),
      };
      buttons_rect = {
          SDL_FRect{bx + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
      };
      break;
    }
    case LoginNoticeSystemInstance::NoticeType::kCharacterDelete: {
      buttons_nodes = {
          WzResource::ui_->find(u"Login.img/Notice/BtYes"),
          WzResource::ui_->find(u"Login.img/Notice/BtNo1"),
      };
      buttons_rect = {
          SDL_FRect{bx - 35 + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
          SDL_FRect{bx + 35 + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
      };
      break;
    }
    case LoginNoticeSystemInstance::NoticeType::kLogining: {
      break;
    }
  }
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x = (int)rect.x;
    rect.y = (int)rect.y;
    Button button{.node = buttons_nodes[i],
                  .rect = rect,
                  .enable = true,
                  .focused = true};
    components_.push_back(button);
    button.Render();
  }
}

std::vector<Button>& LoginNoticeUiSystem::GetComponent() {
  return components_;
}


bool LoginNoticeUiSystem::Render() {
  RenderBackgrnd();
  RenderButton();
  return true;
}

void LoginNoticeUiSystem::EventClose() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

bool LoginNoticeUiSystem::EventButton(SDL_Event* event) {
  const auto w = 1366;
  const auto h = 768;
  std::vector<SDL_FRect> buttons_rect;
  std::vector<void (*)()> fns = {EventClose};
  auto [bx, by] = LoadPos();
  switch (type_) {
    case LoginNoticeSystemInstance::NoticeType::kCharacternameError:
    case LoginNoticeSystemInstance::NoticeType::kCharacterFull:
    case LoginNoticeSystemInstance::NoticeType::kCharacterUseAp:
    case LoginNoticeSystemInstance::NoticeType::kLoginingUsed:
    case LoginNoticeSystemInstance::NoticeType::kCharacternameUsed: {
      buttons_rect = {
          SDL_FRect{bx + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
      };
      break;
    }
    case LoginNoticeSystemInstance::NoticeType::kCharacterDelete: {
      buttons_rect = {
          SDL_FRect{bx - 35 + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
          SDL_FRect{bx + 35 + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
      };
      fns = {fn_, EventClose};
      break;
    }
    default: {
      break;
    }
  }
  for (size_t i = 0; i < components_.size() && i < fns.size(); ++i) {
    components_[i].callback = fns[i];
  }
  if (DispatchButtonClick(components_, true)) {
    return false;
  }
  return true;
}

bool LoginNoticeUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        r = EventButton(event);
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

  return false;
}

bool LoginNoticeUiSystem::Run() {
  return true;
}

}  // namespace sdlms
