#include "context_menu_ui_system.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_info_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "statusbar_ui_system.h"
#include <cstddef>

SDL_FPoint context_menu_ui_system::load_wh() { return {100, 122}; }

bool context_menu_ui_system::cursor_in() {
  bool r = false;
  auto &mouse = window::mouse_pos;
  auto [w, h] = load_wh();
  SDL_FRect pos_rect{pos.x + 3, pos.y, w - 3, h};
  r = SDL_PointInRectFloat(&mouse, &pos_rect);
  return r;
}

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
  std::array buttons_rect = {
      SDL_FRect{pos.x + 9, pos.y + 6, 85, 14},
      SDL_FRect{pos.x + 9, pos.y + 22, 85, 14},
      SDL_FRect{pos.x + 9, pos.y + 38, 85, 14},
      SDL_FRect{pos.x + 9, pos.y + 54, 85, 14},
      SDL_FRect{pos.x + 9, pos.y + 70, 85, 14},
      SDL_FRect{pos.x + 9, pos.y + 86, 85, 14},
      SDL_FRect{pos.x + 9, pos.y + 102, 85, 14},
  };
  std::vector<bool> disable = {
      false, false, false, false, false, false, false,
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
  render_button();
  return true;
}

void context_menu_ui_system::event_button_info() {
  if (client_id == 0) {
    // self
    character_info_ui_system::character = character_game_instance::self;
    character_info_ui_system::close();
    character_info_ui_system::open();
  } else if (character_game_instance::others.contains(client_id)) {
    character_info_ui_system::character =
        character_game_instance::others[client_id].g_character;
    character_info_ui_system::close();
    character_info_ui_system::open();
  } else {
    fbs::ClientCharacterInfoT ct;
    ct.payload = client_id;
    client_request::send_to_host(ct);
  }
}

void context_menu_ui_system::event_button_save() {
  auto name = client_name;
  auto utf8_str = text_game_instance::load_u8str(name);
  SDL_SetClipboardText(utf8_str.c_str());
}

void context_menu_ui_system::event_button_whisper() {}

void context_menu_ui_system::event_button_party() {}

void context_menu_ui_system::event_button_trade() {
  fbs::ClientCharacterTradeT ct;
  ct.to_id = client_id;
  client_request::send_to_host(ct);
}

void context_menu_ui_system::event_button_friend() {}

void context_menu_ui_system::event_button_chat() {}

bool context_menu_ui_system::event_button(SDL_Event *event) {
  std::vector<void (*)()> fns;
  auto &camera = camera_game_instance::camera;
  std::array r = {
      SDL_FRect{pos.x + 9, pos.y + 6, 85, 14},
      SDL_FRect{pos.x + 9, pos.y + 22, 85, 14},
      SDL_FRect{pos.x + 9, pos.y + 38, 85, 14},
      SDL_FRect{pos.x + 9, pos.y + 54, 85, 14},
      SDL_FRect{pos.x + 9, pos.y + 70, 85, 14},
      SDL_FRect{pos.x + 9, pos.y + 86, 85, 14},
      SDL_FRect{pos.x + 9, pos.y + 102, 85, 14},
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

void context_menu_ui_system::event_close() { close(); };

bool context_menu_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      event_close();
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
      if (cursor_game_instance::cursor_ui == render) {
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        event_button(event);
      }
    }
    event_close();
    r = false;
    break;
  }
  default: {
    break;
  }
  }

  return r;
}
void context_menu_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void context_menu_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}