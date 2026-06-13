#include "character_choose_ui_system.h"
#include "SDL3/SDL_events.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

const static auto origin_x = 280;
const static auto origin_y = 82;

SDL_FPoint character_choose_ui_system::load_pos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void character_choose_ui_system::render_button() {
  //   const static std::array buttons_nodes = {
  //       wz_resource::ui->find(u"Basic.img/BtClose"),
  //   };
  //   auto wh = load_wh();
  //   const std::array buttons_rect = {
  //       SDL_FRect{wh.x - 20, 7, 12, 12}, //
  //   };

  //   for (size_t i = 0; i < buttons_nodes.size(); ++i) {
  //     auto k = buttons_nodes[i];
  //     auto pos_rect = buttons_rect[i];
  //     pos_rect.x += pos.x;
  //     pos_rect.y += pos.y;
  //     pos_rect.x = (int)pos_rect.x;
  //     pos_rect.y = (int)pos_rect.y;
  //     auto &mouse_pos = window::mouse_pos;
  //     // 判断按钮是否被遮挡
  //     auto cursor_in = cursor_game_instance::cursor_ui;
  //     if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render)
  //     {
  //       if (window::mouse_state & SDL_BUTTON_LMASK) {
  //         auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
  //         SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
  //       } else {
  //         auto mouse_over =
  //         wz_resource::load_texture(k->find(u"mouseOver/0"));
  //         SDL_RenderTexture(window::renderer, mouse_over, nullptr,
  //         &pos_rect);
  //       }
  //     } else {
  //       auto normal = wz_resource::load_texture(k->find(u"normal/0"));
  //       SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
  //     }
  //   }
}

void character_choose_ui_system::render_banner() {
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/LoginStart/StepBanner/CharSelect"));
  auto pos = load_pos();
  SDL_FRect pos_rect{
      pos.x + 160,
      pos.y + 85,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
}

void character_choose_ui_system::render_backgrnd() {
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/Common/classicFrame"));
  auto pos = load_pos();
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
}

bool character_choose_ui_system::render() {
  render_backgrnd();
  render_button();
  render_banner();
  return true;
}

bool character_choose_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
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
      r = false;
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
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