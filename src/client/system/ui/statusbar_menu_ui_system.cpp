#include "statusbar_menu_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/audio_game_instance.h"
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

SDL_FRect StatusbarMenuUiSystem::load_r() {
  int w;
  int h;
  int x = 0;
  int y = 0;
  switch (menu_type.value()) {
  case MenuEnums::menu: {
    x = 642;
    y = -80;
    w = 93;
    h = 116;
    break;
  }
  case MenuEnums::shortcut: {
    x = 716;
    y = -184;
    w = 93;
    h = 220;
    break;
  }
  }
  auto screen_w = CameraGameInstance::camera.w;
  auto screen_h = CameraGameInstance::camera.h;
  auto base_x = x + (screen_w - 808) / 2;
  auto base_y = y + (screen_h - 73);
  return {base_x, base_y, static_cast<float>(w), static_cast<float>(h)};
}

void StatusbarMenuUiSystem::render_backgrnd() {
  auto [x, y, w, h] = load_r();
  // render backgrnd
  static auto t2 = WzResource::load_texture(
      WzResource::ui->find(u"StatusBar.img/submenu/backgrnd/0"));
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(t2->w),
      static_cast<float>(t2->h),
  };
  SDL_SetTextureAlphaMod(t2, alpha);
  SDL_RenderTexture(Window::renderer, t2, nullptr, &pos_rect);
  static auto c = WzResource::load_texture(
      WzResource::ui->find(u"StatusBar.img/submenu/backgrnd/1"));
  static auto s = WzResource::load_texture(
      WzResource::ui->find(u"StatusBar.img/submenu/backgrnd/2"));
  pos_rect = {
      x,
      y + t2->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t2->h - s->h),
  };
  SDL_SetTextureAlphaMod(c, alpha);
  SDL_RenderTextureTiled(Window::renderer, c, nullptr, 1, &pos_rect);

  pos_rect = {
      x,
      y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_SetTextureAlphaMod(s, alpha);
  SDL_RenderTexture(Window::renderer, s, nullptr, &pos_rect);
}

void StatusbarMenuUiSystem::render_button() {
  auto [x, y, w, h] = load_r();

  std::vector<wz::Node *> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  std::vector<bool> disable;

  switch (menu_type.value()) {
  case MenuEnums::menu: {
    buttons_nodes = {
        WzResource::ui->find(u"StatusBar.img/submenu/menu/button:Channel"),
        WzResource::ui->find(u"StatusBar.img/submenu/menu/button:GameOpt"),
        WzResource::ui->find(u"StatusBar.img/submenu/menu/button:Quit"),
    };
    buttons_rect = {
        SDL_FRect{6, 24, 81, 25}, //
        SDL_FRect{6, 50, 81, 25}, //
        SDL_FRect{6, 76, 81, 25}, //
    };
    disable = {true, false, false};
    break;
  }
  case MenuEnums::shortcut: {
    buttons_nodes = {
        WzResource::ui->find(u"StatusBar.img/submenu/shortcut/button:Item"),
        WzResource::ui->find(u"StatusBar.img/submenu/shortcut/button:Equip"),
        WzResource::ui->find(u"StatusBar.img/submenu/shortcut/button:Stat"),
        WzResource::ui->find(u"StatusBar.img/submenu/shortcut/button:Skill"),
        WzResource::ui->find(u"StatusBar.img/submenu/shortcut/button:Party"),
        WzResource::ui->find(u"StatusBar.img/submenu/shortcut/button:Quest"),
        WzResource::ui->find(
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
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = CursorGameInstance::cursor_ui;
    if (disable[i]) {
      auto disabled = WzResource::load_texture(k->find(u"disabled/0"));
      SDL_SetTextureAlphaMod(disabled, alpha);
      SDL_RenderTexture(Window::renderer, disabled, nullptr, &pos_rect);
    } else if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) &&
               cursor_in == render && !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::load_texture(k->find(u"pressed/0"));
        SDL_SetTextureAlphaMod(pressed, alpha);
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::load_texture(k->find(u"mouseOver/0"));
        SDL_SetTextureAlphaMod(mouse_over, alpha);
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::load_texture(k->find(u"normal/0"));
      SDL_SetTextureAlphaMod(normal, alpha);
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void StatusbarMenuUiSystem::render_title() {
  auto [x, y, w, h] = load_r();
  SDL_Texture *t;
  switch (menu_type.value()) {
  case MenuEnums::menu: {
    t = WzResource::load_texture(
        WzResource::ui->find(u"StatusBar.img/submenu/canvas:menu"));
    break;
  }
  case MenuEnums::shortcut: {
    t = WzResource::load_texture(
        WzResource::ui->find(u"StatusBar.img/submenu/canvas:shortcut"));
    break;
  }
  }
  SDL_SetTextureAlphaMod(t, alpha);
  SDL_FRect pos{x + 6, y + 8, static_cast<float>(t->w),
                static_cast<float>(t->h)};
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos);
}

bool StatusbarMenuUiSystem::render() {
  alpha += Window::delta_time;
  alpha = std::min(alpha, 255);
  render_backgrnd();
  render_button();
  render_title();
  return true;
}

void StatusbarMenuUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    alpha = 0;

    if (!menu_type.has_value()) {
      menu_type = MenuEnums::menu;
    }

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void StatusbarMenuUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);

  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
}

void StatusbarMenuUiSystem::toggle() {
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
    AudioGameInstance::load_audio(u"UI.img/DlgNotice", 0);
  }
}

void StatusbarMenuUiSystem::event_button_cc() { return; }

void StatusbarMenuUiSystem::event_button_option() {
  OptionUiSystem::toggle();
  return;
}

void StatusbarMenuUiSystem::event_button_quit() {
  SceneSystemInstance::quit_prepare();
  return;
}

void StatusbarMenuUiSystem::event_button_item() {
  PackageUiSystem::toggle();
  return;
}

void StatusbarMenuUiSystem::event_button_equip() {
  EquipUiSystem::toggle();
  return;
}

void StatusbarMenuUiSystem::event_button_stat() {
  CharacterStatUiSystem::toggle();
  return;
}

void StatusbarMenuUiSystem::event_button_skill() {
  SkillUiSystem::toggle();
  return;
}

void StatusbarMenuUiSystem::event_button_party() { return; }

void StatusbarMenuUiSystem::event_button_quest() {
  QuestUiSystem::toggle();
  return;
}

void StatusbarMenuUiSystem::event_button_craft() {
  CraftUiSystem::toggle();
  return;
}

bool StatusbarMenuUiSystem::event_button(SDL_Event *event) {
  std::vector<wz::Node *> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  std::vector<void (*)()> fns;

  switch (menu_type.value()) {
  case MenuEnums::menu: {
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
  case MenuEnums::shortcut: {
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
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }
  return false;
}

bool StatusbarMenuUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      return r;
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
      if (CursorGameInstance::cursor_ui == render) {
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    r = false;
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
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

bool StatusbarMenuUiSystem::cursor_in() {
  auto r = load_r();
  auto &mouse = Window::mouse_pos;
  return SDL_PointInRectFloat(&mouse, &r);
}
