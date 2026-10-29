#include "src/client/system/ui/statusbar_menu_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_stat_ui_system.h"
#include "src/client/system/ui/component/ui_component.h"
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

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <algorithm>

namespace sdlms {

SDL_FRect StatusbarMenuUiSystem::LoadR() {
  int w;
  int h;
  int x = 0;
  int y = 0;
  switch (menu_type_.value()) {
    case MenuEnums::kMenu: {
      x = 642;
      y = -80;
      w = 93;
      h = 116;
      break;
    }
    case MenuEnums::kShortcut: {
      x = 716;
      y = -184;
      w = 93;
      h = 220;
      break;
    }
  }
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = x + (screen_w - 808) / 2;
  auto base_y = y + (screen_h - 73);
  return {base_x, base_y, static_cast<float>(w), static_cast<float>(h)};
}

void StatusbarMenuUiSystem::RenderBackgrnd() {
  auto [x, y, w, h] = LoadR();
  // render backgrnd
  static auto t2 = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/submenu/backgrnd/0"));
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(t2->w),
      static_cast<float>(t2->h),
  };
  SDL_SetTextureAlphaMod(t2, alpha_);
  SDL_RenderTexture(Window::renderer_, t2, nullptr, &pos_rect);
  static auto c = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/submenu/backgrnd/1"));
  static auto s = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/submenu/backgrnd/2"));
  pos_rect = {
      x,
      y + t2->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t2->h - s->h),
  };
  SDL_SetTextureAlphaMod(c, alpha_);
  SDL_RenderTextureTiled(Window::renderer_, c, nullptr, 1, &pos_rect);

  pos_rect = {
      x,
      y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_SetTextureAlphaMod(s, alpha_);
  SDL_RenderTexture(Window::renderer_, s, nullptr, &pos_rect);
}

UIComponents& StatusbarMenuUiSystem::GetComponent() {
  components_.clear();
  auto [x, y, w, h] = LoadR();

  std::vector<wz::Node*> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  std::vector<bool> disable;

  switch (menu_type_.value()) {
    case MenuEnums::kMenu: {
      buttons_nodes = {
          WzResource::ui_->find(u"StatusBar.img/submenu/menu/button:Channel"),
          WzResource::ui_->find(u"StatusBar.img/submenu/menu/button:GameOpt"),
          WzResource::ui_->find(u"StatusBar.img/submenu/menu/button:Quit"),
      };
      buttons_rect = {
          SDL_FRect{6, 24, 81, 25},  //
          SDL_FRect{6, 50, 81, 25},  //
          SDL_FRect{6, 76, 81, 25},  //
      };
      disable = {true, false, false};
      break;
    }
    case MenuEnums::kShortcut: {
      buttons_nodes = {
          WzResource::ui_->find(u"StatusBar.img/submenu/shortcut/button:Item"),
          WzResource::ui_->find(u"StatusBar.img/submenu/shortcut/button:Equip"),
          WzResource::ui_->find(u"StatusBar.img/submenu/shortcut/button:Stat"),
          WzResource::ui_->find(u"StatusBar.img/submenu/shortcut/button:Skill"),
          WzResource::ui_->find(u"StatusBar.img/submenu/shortcut/button:Party"),
          WzResource::ui_->find(u"StatusBar.img/submenu/shortcut/button:Quest"),
          WzResource::ui_->find(
              u"StatusBar.img/submenu/shortcut/button:Crafting"),
      };
      buttons_rect = {
          SDL_FRect{6, 24, 81, 25},   //
          SDL_FRect{6, 50, 81, 25},   //
          SDL_FRect{6, 76, 81, 25},   //
          SDL_FRect{6, 102, 81, 25},  //
          SDL_FRect{6, 128, 81, 25},  //
          SDL_FRect{6, 154, 81, 25},  //
          SDL_FRect{6, 180, 81, 25},  //
      };
      disable = {false, false, false, false, false, false, false};
      break;
    }
  }

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x += x;
    rect.y += y;
    rect.x = (int)rect.x;
    rect.y = (int)rect.y;
    Button button{.node = buttons_nodes[i],
                  .rect = rect,
                  .enable = !disable[i],
                  .focused = CursorGameInstance::cursor_ui_ == Render &&
                             !CursorGameInstance::modal_overlay_,
                  .alpha = static_cast<uint8_t>(alpha_)};
    components_.push_back(button);
  }


  switch (menu_type_.value()) {
    case MenuEnums::kMenu: {
      std::get<Button>(components_[0]).callback = EventButtonCc;
      std::get<Button>(components_[1]).callback = EventButtonOption;
      std::get<Button>(components_[2]).callback = EventButtonQuit;
      break;
    }
    case MenuEnums::kShortcut: {
      std::get<Button>(components_[0]).callback = EventButtonItem;
      std::get<Button>(components_[1]).callback = EventButtonEquip;
      std::get<Button>(components_[2]).callback = EventButtonStat;
      std::get<Button>(components_[3]).callback = EventButtonSkill;
      std::get<Button>(components_[4]).callback = EventButtonParty;
      std::get<Button>(components_[5]).callback = EventButtonQuest;
      std::get<Button>(components_[6]).callback = EventButtonCraft;
      break;
    }
  }


  return components_;
}


void StatusbarMenuUiSystem::RenderUI() {
  RenderComponents(GetComponent());
}


void StatusbarMenuUiSystem::RenderTitle() {
  auto [x, y, w, h] = LoadR();
  SDL_Texture* t;
  switch (menu_type_.value()) {
    case MenuEnums::kMenu: {
      t = WzResource::LoadTexture(
          WzResource::ui_->find(u"StatusBar.img/submenu/canvas:menu"));
      break;
    }
    case MenuEnums::kShortcut: {
      t = WzResource::LoadTexture(
          WzResource::ui_->find(u"StatusBar.img/submenu/canvas:shortcut"));
      break;
    }
  }
  SDL_SetTextureAlphaMod(t, alpha_);
  SDL_FRect pos{x + 6, y + 8, static_cast<float>(t->w),
                static_cast<float>(t->h)};
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos);
}

bool StatusbarMenuUiSystem::Render() {
  alpha_ += Window::delta_time_;
  alpha_ = std::min(alpha_, 255);
  RenderBackgrnd();
  RenderUI();
  RenderTitle();
  return true;
}

void StatusbarMenuUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    alpha_ = 0;

    if (!menu_type_.has_value()) {
      menu_type_ = MenuEnums::kMenu;
    }

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void StatusbarMenuUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);

  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
}

void StatusbarMenuUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
    AudioGameInstance::LoadAudio(u"UI.img/DlgNotice", 0);
  }
}

void StatusbarMenuUiSystem::EventButtonCc() {
  return;
}

void StatusbarMenuUiSystem::EventButtonOption() {
  OptionUiSystem::Toggle();
  return;
}

void StatusbarMenuUiSystem::EventButtonQuit() {
  SceneSystemInstance::QuitPrepare();
  return;
}

void StatusbarMenuUiSystem::EventButtonItem() {
  PackageUiSystem::Toggle();
  return;
}

void StatusbarMenuUiSystem::EventButtonEquip() {
  EquipUiSystem::Toggle();
  return;
}

void StatusbarMenuUiSystem::EventButtonStat() {
  CharacterStatUiSystem::Toggle();
  return;
}

void StatusbarMenuUiSystem::EventButtonSkill() {
  SkillUiSystem::Toggle();
  return;
}

void StatusbarMenuUiSystem::EventButtonParty() {
  return;
}

void StatusbarMenuUiSystem::EventButtonQuest() {
  QuestUiSystem::Toggle();
  return;
}

void StatusbarMenuUiSystem::EventButtonCraft() {
  CraftUiSystem::Toggle();
  return;
}

bool StatusbarMenuUiSystem::EventButton(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), false);
}


bool StatusbarMenuUiSystem::Event(SDL_Event* event) {
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
        if (CursorGameInstance::cursor_ui_ == Render) {
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      r = false;
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = !EventButton(event);
        }
        Close();
      }
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

bool StatusbarMenuUiSystem::CursorIn() {
  auto r = LoadR();
  auto& mouse = Window::mouse_pos_;
  return SDL_PointInRectFloat(&mouse, &r);
}

}  // namespace sdlms
