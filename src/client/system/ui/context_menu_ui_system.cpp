#include "context_menu_ui_system.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "statusbar_ui_system.h"
#include <cstddef>

SDL_FPoint context_menu_ui_system::load_wh() { return {100, 200}; }

void context_menu_ui_system::render_backgrnd() {
  auto [w, h] = load_wh();
  static auto t2 = wz_resource::load_texture(
      wz_resource::ui->find(u"ContextMenu.img/ContextMenu/t2"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t2->w),
      static_cast<float>(t2->h),
  };
  SDL_RenderTexture(window::renderer, t2, nullptr, &pos_rect);
  static auto c = wz_resource::load_texture(
      wz_resource::ui->find(u"ContextMenu.img/ContextMenu/c"));
  static auto s = wz_resource::load_texture(
      wz_resource::ui->find(u"ContextMenu.img/ContextMenu/s"));
  pos_rect = {
      pos.x + 3,
      pos.y + t2->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t2->h - s->h),
  };
  SDL_RenderTextureTiled(window::renderer, c, nullptr, 1, &pos_rect);

  pos_rect = {
      pos.x + 3,
      pos.y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_RenderTexture(window::renderer, s, nullptr, &pos_rect);
}

void context_menu_ui_system::render_button() {
  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"ContextMenu.img/ContextMenu/BtInfo"),
      wz_resource::ui->find(u"ContextMenu.img/ContextMenu/BtSave"),
      wz_resource::ui->find(u"ContextMenu.img/ContextMenu/BtWhisper"),
      wz_resource::ui->find(u"ContextMenu.img/ContextMenu/BtParty"),
      wz_resource::ui->find(u"ContextMenu.img/ContextMenu/BtTrade"),
      wz_resource::ui->find(u"ContextMenu.img/ContextMenu/BtFriend"),
      wz_resource::ui->find(u"ContextMenu.img/ContextMenu/BtMapleChat"),
  };
  auto &camera = camera_game_instance::camera;
  std::array buttons_rect = {
      SDL_FRect{146 - camera.x, -656 - camera.y, 129, 41},
      SDL_FRect{146 - camera.x, -607 - camera.y, 129, 45},
      SDL_FRect{146 - camera.x, -544 - camera.y, 129, 55},
  };
  std::vector<bool> disable = {
      false,
      false,
      false,
  };
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = window::mouse_pos;
    if (disable[i]) {
      auto d = wz_resource::load_texture(k->find(u"disabled/0"));
      SDL_RenderTexture(window::renderer, d, nullptr, &pos_rect);
    } else if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
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

bool context_menu_ui_system::render() {
  render_backgrnd();
  return true;
}

void context_menu_ui_system::event_button_info() {
  fbs::ClientCharacterInfoT ct;
  ct.payload = client_id;
  client_request::send_to_host(ct);
}

void context_menu_ui_system::event_button_save() {
  auto name = client_name;
  auto utf8_str = freetype::load_u8str(name);
  SDL_SetClipboardText(utf8_str.c_str());
}

void context_menu_ui_system::event_button_whisper() {}

void context_menu_ui_system::event_button_party() {}

void context_menu_ui_system::event_button_trade() {
  fbs::ClientCharacterTradeT ct;
  ct.to_id = client_id;
  ct.request = true;
  client_request::send_to_host(ct);
}

void context_menu_ui_system::event_button_friend() {}

void context_menu_ui_system::event_button_chat() {}

bool context_menu_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto &camera = camera_game_instance::camera;
  r = {
      SDL_FRect{146 - camera.x, -656 - camera.y, 129, 41},
      SDL_FRect{146 - camera.x, -607 - camera.y, 129, 45},
      SDL_FRect{146 - camera.x, -544 - camera.y, 129, 55},
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
  };
  fns = {
      event_button_info,  event_button_save,  event_button_whisper,
      event_button_party, event_button_trade, event_button_friend,
      event_button_chat,
  };
  std::vector<bool> disable = {
      false,
      false,
      false,
      false,
  };
  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect) && !disable[i]) {
      fns[i]();
      return false;
    }
  }

  return true;
}

bool context_menu_ui_system::event(SDL_Event *event) { return true; }
void context_menu_ui_system::open() {
  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.begin(), event);
}

void context_menu_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}