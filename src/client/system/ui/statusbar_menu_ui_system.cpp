#include "statusbar_menu_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

void statusbar_menu_ui_system::render_backgrnd() {
  int w;
  int h;
  int x = 0;
  switch (menu_type.value()) {
  case menu_enums::menu: {
    w = 93;
    h = 300;
    break;
  }
  case menu_enums::shortcut: {
    w = 93;
    h = 300;
    break;
  }
  }
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  // render backgrnd
  static auto t2 = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/submenu/backgrnd/0"));
  SDL_FRect pos_rect{
      base_x + x,
      base_y,
      static_cast<float>(t2->w),
      static_cast<float>(t2->h),
  };
  SDL_RenderTexture(window::renderer, t2, nullptr, &pos_rect);
  static auto c = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/submenu/backgrnd/1"));
  static auto s = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/submenu/backgrnd/2"));
  pos_rect = {
      base_x + x,
      base_y + t2->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t2->h - s->h),
  };
  SDL_RenderTextureTiled(window::renderer, c, nullptr, 1, &pos_rect);

  pos_rect = {
      base_x + x,
      base_y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_RenderTexture(window::renderer, s, nullptr, &pos_rect);
}

void statusbar_menu_ui_system::render_button() {
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);

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
        SDL_FRect{6, 24, 12, 12}, //
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
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 24, 81, 25}, //
    };
    break;
  }
  }

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += base_x;
    pos_rect.y += base_y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        !cursor_game_instance::modal_overlay) {
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

bool statusbar_menu_ui_system::render() {
  render_backgrnd();
  render_button();
  return true;
}

void statusbar_menu_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
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

bool statusbar_menu_ui_system::event_button(SDL_Event *event) {
  std::vector<wz::Node *> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  std::vector<void (*)()> fns;

  switch (menu_type.value()) {
  case menu_enums::menu: {
    // buttons_nodes = {
    //     wz_resource::ui->find(u"StatusBar.img/submenu/menu/button:Channel"),
    //     wz_resource::ui->find(u"StatusBar.img/submenu/menu/button:GameOpt"),
    //     wz_resource::ui->find(u"StatusBar.img/submenu/menu/button:Quit"),
    // };
    buttons_rect = {
        SDL_FRect{6, 24, 12, 12}, //
    };
    break;
  }
  case menu_enums::shortcut: {
    // buttons_nodes = {
    //     wz_resource::ui->find(u"StatusBar.img/submenu/shortcut/button:Item"),
    //     wz_resource::ui->find(u"StatusBar.img/submenu/shortcut/button:Equip"),
    //     wz_resource::ui->find(u"StatusBar.img/submenu/shortcut/button:Stat"),
    //     wz_resource::ui->find(u"StatusBar.img/submenu/shortcut/button:Skill"),
    //     wz_resource::ui->find(u"StatusBar.img/submenu/shortcut/button:Party"),
    //     wz_resource::ui->find(u"StatusBar.img/submenu/shortcut/button:Quest"),
    //     wz_resource::ui->find(
    //         u"StatusBar.img/submenu/shortcut/button:Crafting"),
    // };
    buttons_rect = {
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 24, 81, 25}, //
    };
    break;
  }
  }
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += base_x;
    pos_rect.y += base_y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }
  return false;
}

bool statusbar_menu_ui_system::event(SDL_Event *event) { return true; }