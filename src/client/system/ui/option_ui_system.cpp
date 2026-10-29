#include "src/client/system/ui/option_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/button.h"
#include "src/client/system/ui/component/combo_box.h"
#include "src/client/system/ui/component/slider.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>

#include <string>
#include <vector>

namespace sdlms {

enum class OptionType {
  kGraphic,
  kSound,
  kGame,
  kSocial,
  kController,
};

void OptionUiSystem::RenderBackgrnd() {
  static auto texture =
      WzResource::LoadTexture(WzResource::ui_->find(u"Options.img/backgrnd"));
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

void OptionUiSystem::RenderOption() {
  std::vector<wz::Node*> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  auto top = CursorGameInstance::cursor_ui_ == Render;

  switch (static_cast<OptionType>(active_tab_)) {
    case OptionType::kController: {
      break;
    }
    case OptionType::kGraphic: {
      static auto backgrnd = WzResource::LoadTexture(
          WzResource::ui_->find(u"Options.img/graphic/layer:backgrnd"));
      SDL_FRect pos_rect{
          pos_.x + 9,
          pos_.y + 53,
          static_cast<float>(backgrnd->w),
          static_cast<float>(backgrnd->h),
      };
      SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
      break;
    }
    case OptionType::kSound: {
      static auto backgrnd = WzResource::LoadTexture(
          WzResource::ui_->find(u"Options.img/sound/layer:backgrnd"));
      SDL_FRect pos_rect{
          pos_.x + 9,
          pos_.y + 53,
          static_cast<float>(backgrnd->w),
          static_cast<float>(backgrnd->h),
      };
      SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
      // master volume
      // hp warn
      Freetype::LoadAligned(true);
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"Master Volume", pos_.x + 12, pos_.y + 53, 105, 1);
      Freetype::LoadColor(0, 0, 0, 255);

      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 91);
      Slider(hp_warn_, 100, 167).Render(pos_.x + 163, pos_.y + 95);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 91);
      SDL_Texture* t;
      if (master_volume_mute_) {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/1"));
      } else {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/0"));
      }
      pos_rect.x = pos_.x + 1;
      pos_rect.y = pos_.y + 1;
      pos_rect.w = t->w;
      pos_rect.h = t->h;
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      Freetype::LoadAligned(true);
      Freetype::LoadSize(12);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(u"Mute", pos_.x, pos_.y);

      // BGM
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"BGM", pos_.x + 12, pos_.y + 53, 105, 1);
      Freetype::LoadColor(0, 0, 0, 255);

      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 91);
      Slider(hp_warn_, 100, 167).Render(pos_.x + 163, pos_.y + 95);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 91);
      if (bgm_volume_mute_) {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/1"));
      } else {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/0"));
      }
      pos_rect.x = pos_.x + 1;
      pos_rect.y = pos_.y + 1;
      pos_rect.w = t->w;
      pos_rect.h = t->h;
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      Freetype::LoadAligned(true);
      Freetype::LoadSize(12);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(u"Mute", pos_.x, pos_.y);

      // Normal SFX
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"Normal SFX", pos_.x + 12, pos_.y + 53, 105, 1);
      Freetype::LoadColor(0, 0, 0, 255);

      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 91);
      Slider(hp_warn_, 100, 167).Render(pos_.x + 163, pos_.y + 95);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 91);
      if (normal_volume_mute_) {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/1"));
      } else {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/0"));
      }
      pos_rect.x = pos_.x + 1;
      pos_rect.y = pos_.y + 1;
      pos_rect.w = t->w;
      pos_rect.h = t->h;
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      Freetype::LoadAligned(true);
      Freetype::LoadSize(12);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(u"Mute", pos_.x, pos_.y);

      // Monster SFX
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"Monster SFX", pos_.x + 12, pos_.y + 53, 105, 1);
      Freetype::LoadColor(0, 0, 0, 255);

      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 91);
      Slider(hp_warn_, 100, 167).Render(pos_.x + 163, pos_.y + 95);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 91);
      if (monster_volume_mute_) {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/1"));
      } else {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/0"));
      }
      pos_rect.x = pos_.x + 1;
      pos_rect.y = pos_.y + 1;
      pos_rect.w = t->w;
      pos_rect.h = t->h;
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      Freetype::LoadAligned(true);
      Freetype::LoadSize(12);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(u"Mute", pos_.x, pos_.y);

      // Skill SFX
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"Skill SFX", pos_.x + 12, pos_.y + 53, 105, 1);
      Freetype::LoadColor(0, 0, 0, 255);

      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 91);
      Slider(hp_warn_, 100, 167).Render(pos_.x + 163, pos_.y + 95);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 91);
      if (skill_volume_mute_) {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/1"));
      } else {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/0"));
      }
      pos_rect.x = pos_.x + 1;
      pos_rect.y = pos_.y + 1;
      pos_rect.w = t->w;
      pos_rect.h = t->h;
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      Freetype::LoadAligned(true);
      Freetype::LoadSize(12);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(u"Mute", pos_.x, pos_.y);
      break;
    }
    case OptionType::kGame: {
      static auto backgrnd = WzResource::LoadTexture(
          WzResource::ui_->find(u"Options.img/game/layer:backgrnd"));
      SDL_FRect pos_rect{
          pos_.x + 9,
          pos_.y + 53,
          static_cast<float>(backgrnd->w),
          static_cast<float>(backgrnd->h),
      };
      SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
      // hp warn
      Freetype::LoadAligned(true);
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"HP Warning", pos_.x + 12, pos_.y + 53, 105, 1.3);
      Freetype::LoadColor(0, 0, 0, 255);

      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 58);
      // hp slider
      Slider(hp_warn_, 100, 167).Render(pos_.x + 163, pos_.y + 63);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 58);

      // mp warn
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"MP Warning", pos_.x + 12, pos_.y + 86, 105, 1.3);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 91);
      Slider(hp_warn_, 100, 167).Render(pos_.x + 163, pos_.y + 95);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 91);

      // enemy info
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"Enemy Info", pos_.x + 12, pos_.y + 119, 105, 1.3);
      Freetype::LoadColor(0, 0, 0, 255);
      enemy_info_cm_.Render(pos_.x + 131, pos_.y + 124, top, false);
      // screen format
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"ScreenShot File\n Format", pos_.x + 12, pos_.y + 158,
                         105, 1);
      Freetype::LoadColor(0, 0, 0, 255);
      screenshot_format_.Render(pos_.x + 131, pos_.y + 164, top, false);
      // screen folder
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"ScreenShot File\n Format", pos_.x + 12, pos_.y + 158,
                         105, 1);
      Freetype::LoadColor(0, 0, 0, 255);

      screenshot_location_.Render(pos_.x + 131, pos_.y + 197, top, false);
      // burst screen shot
      SDL_Texture* t;
      if (continue_capture_) {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/1"));
      } else {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/0"));
      }
      pos_rect.x = pos_.x + 1;
      pos_rect.y = pos_.y + 1;
      pos_rect.w = t->w;
      pos_rect.h = t->h;
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      std::u16string text = u"Continuous Capture";
      Freetype::LoadSize(12);
      Freetype::DrawLine(text, pos_rect.x + 100, pos_rect.y);

      auto tmp = std::to_string(continue_capture_num_);
      text = std::u16string{tmp.begin(), tmp.end()} + u"   Shots";
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(text, pos_rect.x + 100, pos_rect.y);

      // buttons_nodes = {
      //     wz_resource::ui->find(u"Basic.img/BtMax"),
      //     wz_resource::ui->find(u"Basic.img/BtMin"),
      //     wz_resource::ui->find(u"Basic.img/BtMax"),
      //     wz_resource::ui->find(u"Basic.img/BtMin"),
      // };
      buttons_rect = {

      };

      text = u"2 to 10 Shots Avaliable";
      Freetype::DrawLine(text, pos_.x, pos_.y);

      if (auto_capture_) {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/1"));
      } else {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/0"));
      }
      pos_rect.x = pos_.x + 1;
      pos_rect.y = pos_.y + 1;
      pos_rect.w = t->w;
      pos_rect.h = t->h;
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      text = u"Auto-Capture Memorable Moments";

      if (memory_map_) {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/1"));
      } else {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/0"));
      }
      pos_rect.x = pos_.x + 1;
      pos_rect.y = pos_.y + 1;
      pos_rect.w = t->w;
      pos_rect.h = t->h;
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      text = u"Use";

      tmp = std::to_string(memory_limit_);
      text = std::u16string{tmp.begin(), tmp.end()} + u"   Shots";
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(text, pos_rect.x + 100, pos_rect.y);

      break;
    }
    case OptionType::kSocial: {
      static auto backgrnd = WzResource::LoadTexture(
          WzResource::ui_->find(u"Options.img/game/layer:backgrnd"));
      SDL_FRect pos_rect{
          pos_.x + 9,
          pos_.y + 53,
          static_cast<float>(backgrnd->w),
          static_cast<float>(backgrnd->h),
      };
      SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);

      SDL_Texture* t;
      // whisper
      if (whisper_) {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/1"));
      } else {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/0"));
      }
      pos_rect.x = pos_.x + 1;
      pos_rect.y = pos_.y + 1;
      pos_rect.w = t->w;
      pos_rect.h = t->h;
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      std::u16string text = u"Continuous Capture";
      Freetype::LoadSize(12);
      Freetype::DrawLine(text, pos_rect.x + 100, pos_rect.y);
    }
  }
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto& mouse_pos = Window::mouse_pos_;
    // 判断按钮是否被遮挡
    auto cursor_in = CursorGameInstance::cursor_ui_;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == Render &&
        !CursorGameInstance::modal_overlay_) {
      if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::LoadTexture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer_, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::LoadTexture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer_, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::LoadTexture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer_, normal, nullptr, &pos_rect);
    }
  }
}

void OptionUiSystem::RenderButton() {
  for (Button& button : GetComponent()) {
    button.Render();
  }
}


std::vector<Button>& OptionUiSystem::GetComponent() {
  const static std::array<Button::Callback, 1> kCallbacks = {
      EventButtonOk,
  };

  components_.clear();
  const static std::array buttons_node = {
      WzResource::ui_->find(u"KeyBindings.img/quickslot/button:confirm"),
      WzResource::ui_->find(u"ChannelChange.img/Channel/BtCancel"),
      WzResource::ui_->find(u"Basic.img/BtClose"),
  };
  const std::array buttons_rect = {
      SDL_FRect{176, 407, 53, 18},  //
      SDL_FRect{232, 407, 47, 18},  //
      SDL_FRect{266, 7, 12, 12},    //
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x += (int)pos_.x;
    rect.y += (int)pos_.y;
    Button button{.node = buttons_node[i],
                  .rect = rect,
                  .enable = true,
                  .focused = CursorGameInstance::cursor_ui_ == Render &&
                             !CursorGameInstance::modal_overlay_};
    components_.push_back(button);
    if (i < kCallbacks.size()) {
      components_.back().callback = kCallbacks[i];
    }
  }
  return components_;
}


void OptionUiSystem::RenderTab() {
  const static std::array tab_pos = {
      SDL_FPoint{10, 24},   //
      SDL_FPoint{81, 24},   //
      SDL_FPoint{152, 24},  //
      SDL_FPoint{223, 24},  //
      SDL_FPoint{294, 24},  //
  };
  const static auto tab_node = WzResource::ui_->find(u"Options.img/tab");
  const static std::array active_texture = {
      WzResource::LoadTexture(tab_node->find(u"enabled/0")),
      WzResource::LoadTexture(tab_node->find(u"enabled/1")),
      WzResource::LoadTexture(tab_node->find(u"enabled/2")),
      WzResource::LoadTexture(tab_node->find(u"enabled/3")),
      WzResource::LoadTexture(tab_node->find(u"enabled/4")),
  };
  const static std::array disabled_texture = {
      WzResource::LoadTexture(tab_node->find(u"disabled/0")),
      WzResource::LoadTexture(tab_node->find(u"disabled/1")),
      WzResource::LoadTexture(tab_node->find(u"disabled/2")),
      WzResource::LoadTexture(tab_node->find(u"disabled/3")),
      WzResource::LoadTexture(tab_node->find(u"disabled/4")),
  };
  for (uint8_t i = 0; i < tab_pos.size(); i++) {
    SDL_Texture* t = active_tab_ == i ? active_texture[i] : disabled_texture[i];
    SDL_FRect pos_rect{
        static_cast<float>(int(pos_.x + tab_pos[i].x)),
        static_cast<float>(int(pos_.y + tab_pos[i].y)),
        static_cast<float>(t->w),
        static_cast<float>(t->h),
    };
    SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
  }
}

bool OptionUiSystem::Render() {
  RenderBackgrnd();
  RenderButton();
  RenderTab();
  RenderOption();
  return true;
}

SDL_FPoint OptionUiSystem::LoadWh() {
  return {287, 432};
}

void OptionUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    active_tab_ = static_cast<uint8_t>(OptionType::kGame);

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void OptionUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void OptionUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void OptionUiSystem::EventDragStart(SDL_Event* event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
  }
  return;
}

void OptionUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

void OptionUiSystem::EventDragMove(SDL_Event* event) {
  if (drag_.has_value()) {
    pos_ = {event->motion.x + drag_->x, event->motion.y + drag_->y};
    auto& camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  }
  return;
}

void OptionUiSystem::EventButtonOk() {
  Close();
}

void OptionUiSystem::EventTab(SDL_Event* event) {
  const static std::array tab_rect = {
      SDL_FRect{5, 26, 33, 19},   //
      SDL_FRect{38, 26, 33, 19},  //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      active_tab_ = i;
    }
  }
}

bool OptionUiSystem::EventButton(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), false);
}


void OptionUiSystem::EventMotionBtAudio(SDL_Event* event) {
  static int8_t bt_audio = -1;
  int8_t bt_in = -1;
  auto& components = GetComponent();
  for (size_t i = 0; i < components.size(); ++i) {
    if (components[i].Hover()) {
      bt_in = static_cast<int8_t>(i);
      break;
    }
  }
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool OptionUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          EventTop();
          EventDragStart(event);
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = !EventButton(event);
        }
        EventDragEnd();
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventMotionBtAudio(event);
      EventDragMove(event);
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

void OptionUiSystem::Toggle() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool OptionUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

}  // namespace sdlms
