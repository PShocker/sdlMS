#include "login_notice_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include <string>
#include <vector>

SDL_FPoint LoginNoticeUiSystem::load_pos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto &camera = CameraGameInstance::camera;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void LoginNoticeUiSystem::render_backgrnd() {
  const auto w = 1366;
  const auto h = 768;
  SDL_Texture *backgrnd;
  SDL_Texture *text;
  SDL_FPoint text_pos;
  static auto notice_node = WzResource::ui->find(u"Login.img/Notice");
  auto [bx, by] = load_pos();
  switch (type) {
  case LoginNoticeSystemInstance::NoticeType::kCharacternameError: {
    backgrnd = WzResource::load_texture(notice_node->find(u"backgrnd/0"));
    text =
        WzResource::load_texture(notice_node->find(u"text/cannotUseThisName"));
    text_pos = {17, 13};
    break;
  }
  case LoginNoticeSystemInstance::NoticeType::kCharacternameUsed: {
    backgrnd = WzResource::load_texture(notice_node->find(u"backgrnd/0"));
    text = WzResource::load_texture(notice_node->find(u"text/8"));
    text_pos = {17, 13};
    break;
  }
  case LoginNoticeSystemInstance::NoticeType::kCharacterDelete: {
    backgrnd = WzResource::load_texture(notice_node->find(u"backgrnd/0"));
    text = WzResource::load_texture(notice_node->find(u"text/58"));
    text_pos = {17, 13};
    break;
  }
  case LoginNoticeSystemInstance::NoticeType::kCharacterFull: {
    backgrnd = WzResource::load_texture(notice_node->find(u"backgrnd/0"));
    text = WzResource::load_texture(
        notice_node->find(u"text/cannotCreateAccountMore"));
    text_pos = {17, 13};
    break;
  }
  case LoginNoticeSystemInstance::NoticeType::kCharacterUseAp: {
    backgrnd = WzResource::load_texture(notice_node->find(u"backgrnd/0"));
    text = WzResource::load_texture(notice_node->find(u"text/useAllAP"));
    text_pos = {17, 13};
    break;
  }
  case LoginNoticeSystemInstance::NoticeType::kLogining: {
    backgrnd =
        WzResource::load_texture(notice_node->find(u"Loading/backgrnd"));
    text = nullptr;
    break;
  }
  case LoginNoticeSystemInstance::NoticeType::kLoginingUsed: {
    backgrnd =
        WzResource::load_texture(notice_node->find(u"Loading/backgrnd"));
    text = WzResource::load_texture(notice_node->find(u"text/8"));
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
  SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);

  if (text != nullptr) {
    pos_rect.x += text_pos.x;
    pos_rect.y += text_pos.y;
    pos_rect.w = text->w;
    pos_rect.h = text->h;
    SDL_RenderTexture(Window::renderer, text, nullptr, &pos_rect);
  }
  switch (type) {
  case LoginNoticeSystemInstance::NoticeType::kLogining: {
    static auto circle_node =
        WzResource::ui->find(u"Login.img/Notice/Loading/circle");
    const auto circle_delay = 90;
    auto now = Window::dt_now;
    auto index = (now / circle_delay) % (circle_node->children_count());
    auto circle_texture = circle_node->get_child(std::to_string(index));
    auto circle = WzResource::load_texture(circle_texture);
    pos_rect.x += 84;
    pos_rect.y += 40;
    pos_rect.w = circle->w;
    pos_rect.h = circle->h;
    SDL_RenderTexture(Window::renderer, circle, nullptr, &pos_rect);
    break;
  }
  default: {
    break;
  }
  }
}

void LoginNoticeUiSystem::render_button() {
  const auto w = 1366;
  const auto h = 768;
  std::vector<wz::Node *> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  auto [bx, by] = load_pos();
  switch (type) {
  case LoginNoticeSystemInstance::NoticeType::kCharacternameError:
  case LoginNoticeSystemInstance::NoticeType::kCharacterFull:
  case LoginNoticeSystemInstance::NoticeType::kCharacterUseAp:
  case LoginNoticeSystemInstance::NoticeType::kLoginingUsed:
  case LoginNoticeSystemInstance::NoticeType::kCharacternameUsed: {
    buttons_nodes = {
        WzResource::ui->find(u"Login.img/Notice/BtYes"),
    };
    buttons_rect = {
        SDL_FRect{bx + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
    };
    break;
  }
  case LoginNoticeSystemInstance::NoticeType::kCharacterDelete: {
    buttons_nodes = {
        WzResource::ui->find(u"Login.img/Notice/BtYes"),
        WzResource::ui->find(u"Login.img/Notice/BtNo1"),
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
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
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

bool LoginNoticeUiSystem::render() {
  render_backgrnd();
  render_button();
  return true;
}

void LoginNoticeUiSystem::event_close() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
}

bool LoginNoticeUiSystem::event_button(SDL_Event *event) {
  const auto w = 1366;
  const auto h = 768;
  std::vector<SDL_FRect> buttons_rect;
  std::vector<void (*)()> fns = {event_close};
  auto [bx, by] = load_pos();
  switch (type) {
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
    fns = {fn, event_close};
    break;
  }
  default: {
    break;
  }
  }
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
      fns[i]();
      return false;
    }
  }
  return true;
}

bool LoginNoticeUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      r = event_button(event);
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

bool LoginNoticeUiSystem::run() { return true; }