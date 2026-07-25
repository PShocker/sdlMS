#include "statusbar_menu_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_stat_ui_system.h"
#include "src/client/system/ui/craft_ui_system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/option_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/quest_ui_system.h"
#include "src/client/system/ui/skill_ui_system.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>

SDL_FRect statusbar_menu_ui_system::load_r() {
  int w;
  int h;
  int x = 0;
  int y = 0;
  switch (menu_type.value()) {
  case menu_enums::menu: {
    x = 642;
    y = -80;
    w = 93;
    h = 116;
    break;
  }
  case menu_enums::shortcut: {
    x = 716;
    y = -184;
    w = 93;
    h = 220;
    break;
  }
  }
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = x + (screen_w - 808) / 2;
  auto base_y = y + (screen_h - 73);
  return {base_x, base_y, static_cast<float>(w), static_cast<float>(h)};
}

void statusbar_menu_ui_system::render_backgrnd() {
  auto [x, y, w, h] = load_r();
  // render backgrnd
  static auto t2 = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/submenu/backgrnd/0"));
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(t2->w),
      static_cast<float>(t2->h),
  };
  SDL_SetTextureAlphaMod(t2, alpha);
  SDL_RenderTexture(window::renderer, t2, nullptr, &pos_rect);
  static auto c = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/submenu/backgrnd/1"));
  static auto s = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/submenu/backgrnd/2"));
  pos_rect = {
      x,
      y + t2->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t2->h - s->h),
  };
  SDL_SetTextureAlphaMod(c, alpha);
  SDL_RenderTextureTiled(window::renderer, c, nullptr, 1, &pos_rect);

  pos_rect = {
      x,
      y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_SetTextureAlphaMod(s, alpha);
  SDL_RenderTexture(window::renderer, s, nullptr, &pos_rect);
}

void statusbar_menu_ui_system::render_button() {
  auto [x, y, w, h] = load_r();

  std::vector<wz::Node *> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  std::vector<bool> disable;

  switch (menu_type.value()) {
  case menu_enums::menu: {
    buttons_nodes = {
        wz_resource::ui->find(u"StatusBar.img/submenu/menu/button:Channel"),
        wz_resource::ui->find(u"StatusBar.img/submenu/menu/button:GameOpt"),
        wz_resource::ui->find(u"StatusBar.img/submenu/menu/button:Quit"),
    };
    buttons_rect = {
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 50, 81, 25}, //
        SDL_FRect{6, 76, 81, 25}, //
    };
    disable = {true, false, false};
    break;
  }
  case menu_enums::shortcut: {
    buttons_nodes = {
        wz_resource::ui->find(u"StatusBar.img/submenu/shortcut/button:Item"),
        wz_resource::ui->find(u"StatusBar.img/submenu/shortcut/button:Equip"),
        wz_resource::ui->find(u"StatusBar.img/submenu/shortcut/button:Stat"),
        wz_resource::ui->find(u"StatusBar.img/submenu/shortcut/button:Skill"),
        wz_resource::ui->find(u"StatusBar.img/submenu/shortcut/button:Party"),
        wz_resource::ui->find(u"StatusBar.img/submenu/shortcut/button:Quest"),
        wz_resource::ui->find(
            u"StatusBar.img/submenu/shortcut/button:Crafting"),
    };
    buttons_rect = {
        SDL_FRect{6, 24, 81, 25},  //
        SDL_FRect{6, 50, 81, 25},  //
        SDL_FRect{6, 76, 81, 25},  //
        SDL_FRect{6, 102, 81, 25}, //
        SDL_FRect{6, 128, 81, 25}, //
        SDL_FRect{6, 154, 81, 25}, //
        SDL_FRect{6, 180, 81, 25}, //
    };
    disable = {false, false, false, false, false, false, false};
    break;
  }
  }

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += x;
    pos_rect.y += y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (disable[i]) {
      auto disabled = wz_resource::load_texture(k->find(u"disabled/0"));
      SDL_SetTextureAlphaMod(disabled, alpha);
      SDL_RenderTexture(window::renderer, disabled, nullptr, &pos_rect);
    } else if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) &&
               cursor_in == render && !cursor_game_instance::modal_overlay) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
        SDL_SetTextureAlphaMod(pressed, alpha);
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
        SDL_SetTextureAlphaMod(mouse_over, alpha);
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = wz_resource::load_texture(k->find(u"normal/0"));
      SDL_SetTextureAlphaMod(normal, alpha);
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void statusbar_menu_ui_system::render_title() {
  auto [x, y, w, h] = load_r();
  SDL_Texture *t;
  switch (menu_type.value()) {
  case menu_enums::menu: {
    t = wz_resource::load_texture(
        wz_resource::ui->find(u"StatusBar.img/submenu/canvas:menu"));
    break;
  }
  case menu_enums::shortcut: {
    t = wz_resource::load_texture(
        wz_resource::ui->find(u"StatusBar.img/submenu/canvas:shortcut"));
    break;
  }
  }
  SDL_SetTextureAlphaMod(t, alpha);
  SDL_FRect pos{x + 6, y + 8, static_cast<float>(t->w),
                static_cast<float>(t->h)};
  SDL_RenderTexture(window::renderer, t, nullptr, &pos);
}

bool statusbar_menu_ui_system::render() {
  alpha += window::delta_time;
  alpha = std::min(alpha, 255);
  render_backgrnd();
  render_button();
  render_title();
  return true;
}

void statusbar_menu_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    alpha = 0;
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void statusbar_menu_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void statusbar_menu_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void statusbar_menu_ui_system::event_button_cc() { return; }

void statusbar_menu_ui_system::event_button_option() {
  option_ui_system::toggle();
  return;
}

void statusbar_menu_ui_system::event_button_quit() {
  scene_system_instance::quit_prepare();
  return;
}

void statusbar_menu_ui_system::event_button_item() {
  package_ui_system::toggle();
  return;
}

void statusbar_menu_ui_system::event_button_equip() {
  equip_ui_system::toggle();
  return;
}

void statusbar_menu_ui_system::event_button_stat() {
  character_stat_ui_system::toggle();
  return;
}

void statusbar_menu_ui_system::event_button_skill() {
  skill_ui_system::toggle();
  return;
}

void statusbar_menu_ui_system::event_button_party() { return; }

void statusbar_menu_ui_system::event_button_quest() {
  quest_ui_system::toggle();
  return;
}

void statusbar_menu_ui_system::event_button_craft() {
  craft_ui_system::toggle();
  return;
}

bool statusbar_menu_ui_system::event_button(SDL_Event *event) {
  std::vector<wz::Node *> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  std::vector<void (*)()> fns;

  switch (menu_type.value()) {
  case menu_enums::menu: {
    buttons_rect = {
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 50, 81, 25}, //
        SDL_FRect{6, 76, 81, 25}, //
    };
    fns = {
        event_button_cc,
        event_button_option,
        event_button_quit,
    };
    break;
  }
  case menu_enums::shortcut: {
    fns = {
        event_button_item,  event_button_equip, event_button_stat,
        event_button_skill, event_button_party, event_button_quest,
        event_button_craft,
    };
    buttons_rect = {
        SDL_FRect{6, 24, 81, 25},  //
        SDL_FRect{6, 50, 81, 25},  //
        SDL_FRect{6, 76, 81, 25},  //
        SDL_FRect{6, 102, 81, 25}, //
        SDL_FRect{6, 128, 81, 25}, //
        SDL_FRect{6, 154, 81, 25}, //
        SDL_FRect{6, 180, 81, 25}, //
    };
    break;
  }
  }
  auto [x, y, w, h] = load_r();

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += x;
    pos_rect.y += y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }
  return false;
}

bool statusbar_menu_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      close();
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
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    r = false;
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        r = !event_button(event);
      }
      close();
    }
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

bool statusbar_menu_ui_system::cursor_in() {
  auto r = load_r();
  auto &mouse = window::mouse_pos;
  return SDL_PointInRectFloat(&mouse, &r);
}
