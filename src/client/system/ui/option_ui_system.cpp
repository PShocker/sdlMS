#include "src/client/system/ui/option_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/ui_component.h"
#include "src/client/system/ui/component/check_box.h"
#include "src/client/system/ui/component/combo_box.h"
#include "src/client/system/ui/component/slider.h"
#include "src/client/system/ui/component/tab.h"
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

  // 当前标签页的内容区背景
  SDL_Texture* layer = nullptr;
  switch (static_cast<OptionType>(active_tab_)) {
    case OptionType::kGraphic: {
      static auto t = WzResource::LoadTexture(
          WzResource::ui_->find(u"Options.img/graphic/layer:backgrnd"));
      layer = t;
      break;
    }
    case OptionType::kSound: {
      static auto t = WzResource::LoadTexture(
          WzResource::ui_->find(u"Options.img/sound/layer:backgrnd"));
      layer = t;
      break;
    }
    case OptionType::kGame:
    case OptionType::kSocial: {
      static auto t = WzResource::LoadTexture(
          WzResource::ui_->find(u"Options.img/game/layer:backgrnd"));
      layer = t;
      break;
    }
    default: {
      return;
    }
  }
  SDL_FRect layer_rect{
      pos_.x + 9,
      pos_.y + 53,
      static_cast<float>(layer->w),
      static_cast<float>(layer->h),
  };
  SDL_RenderTexture(Window::renderer_, layer, nullptr, &layer_rect);
}

void OptionUiSystem::RenderOption() {
  switch (static_cast<OptionType>(active_tab_)) {
    case OptionType::kSound: {
      Freetype::LoadAligned(true);
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"Master Volume", pos_.x + 12, pos_.y + 53, 105, 1);
      Freetype::LoadColor(0, 0, 0, 255);

      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 91);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 91);
      Freetype::LoadAligned(true);
      Freetype::LoadSize(12);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(u"Mute", pos_.x, pos_.y);

      // BGM
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"BGM", pos_.x + 12, pos_.y + 53, 105, 1);
      Freetype::LoadColor(0, 0, 0, 255);

      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 91);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 91);
      Freetype::LoadAligned(true);
      Freetype::LoadSize(12);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(u"Mute", pos_.x, pos_.y);

      // Normal SFX
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"Normal SFX", pos_.x + 12, pos_.y + 53, 105, 1);
      Freetype::LoadColor(0, 0, 0, 255);

      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 91);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 91);
      Freetype::LoadAligned(true);
      Freetype::LoadSize(12);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(u"Mute", pos_.x, pos_.y);

      // Monster SFX
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"Monster SFX", pos_.x + 12, pos_.y + 53, 105, 1);
      Freetype::LoadColor(0, 0, 0, 255);

      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 91);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 91);
      Freetype::LoadAligned(true);
      Freetype::LoadSize(12);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(u"Mute", pos_.x, pos_.y);

      // Skill SFX
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"Skill SFX", pos_.x + 12, pos_.y + 53, 105, 1);
      Freetype::LoadColor(0, 0, 0, 255);

      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 91);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 91);
      Freetype::LoadAligned(true);
      Freetype::LoadSize(12);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(u"Mute", pos_.x, pos_.y);
      break;
    }
    case OptionType::kGame: {
      // hp warn
      Freetype::LoadAligned(true);
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"HP Warning", pos_.x + 12, pos_.y + 53, 105, 1.3);
      Freetype::LoadColor(0, 0, 0, 255);

      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 58);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 58);

      // mp warn
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"MP Warning", pos_.x + 12, pos_.y + 86, 105, 1.3);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(u"0%", pos_.x + 136, pos_.y + 91);
      Freetype::DrawLine(u"100%", pos_.x + 335, pos_.y + 91);

      // enemy info / screenshot
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"Enemy Info", pos_.x + 12, pos_.y + 119, 105, 1.3);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"ScreenShot File\n Format", pos_.x + 12, pos_.y + 158,
                         105, 1);
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawCstr(u"ScreenShot File\n Format", pos_.x + 12, pos_.y + 158,
                         105, 1);
      Freetype::LoadColor(0, 0, 0, 255);

      std::u16string text = u"Continuous Capture";
      Freetype::LoadSize(12);
      Freetype::DrawLine(text, pos_.x + 101, pos_.y + 1);

      auto tmp = std::to_string(continue_capture_num_);
      text = std::u16string{tmp.begin(), tmp.end()} + u"   Shots";
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(text, pos_.x + 101, pos_.y + 1);

      text = u"2 to 10 Shots Avaliable";
      Freetype::DrawLine(text, pos_.x, pos_.y);

      text = u"Auto-Capture Memorable Moments";

      text = u"Use";

      tmp = std::to_string(memory_limit_);
      text = std::u16string{tmp.begin(), tmp.end()} + u"   Shots";
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::DrawLine(text, pos_.x + 101, pos_.y + 1);
      break;
    }
    case OptionType::kSocial: {
      std::u16string text = u"Continuous Capture";
      Freetype::LoadSize(12);
      Freetype::DrawLine(text, pos_.x + 101, pos_.y + 1);
      break;
    }
    default: {
      break;
    }
  }
}

void OptionUiSystem::RenderUI() {
  RenderComponents(GetComponent());
}


UIComponents& OptionUiSystem::GetComponent() {
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
      std::get<Button>(components_.back()).callback = kCallbacks[i];
    }
  }

  const static auto tab_node = WzResource::ui_->find(u"Options.img/tab");
  std::vector<SDL_FRect> tab_rects;
  for (uint8_t i = 0; i < 5; i++) {
    tab_rects.push_back(
        SDL_FRect{static_cast<float>(int(pos_.x + 10 + i * 71)),
                  static_cast<float>(int(pos_.y + 24)), 71, 21});
  }
  components_.push_back(
      Tab{tab_node, u"enabled", u"disabled", tab_rects, active_tab_});

  switch (static_cast<OptionType>(active_tab_)) {
    case OptionType::kSound: {
      for (int i = 0; i < 5; i++) {
        Slider slider(hp_warn_, 100, 167);
        slider.pos = {pos_.x + 163, pos_.y + 95};
        components_.push_back(slider);
      }
      const bool mute_flags[] = {
          master_volume_mute_,  bgm_volume_mute_,
          normal_volume_mute_,  monster_volume_mute_,
          skill_volume_mute_,
      };
      for (bool checked : mute_flags) {
        CheckBox box{.rect = SDL_FRect{pos_.x + 1, pos_.y + 1},
                     .checked = checked};
        components_.push_back(box);
      }
      break;
    }
    case OptionType::kGame: {
      Slider hp_slider(hp_warn_, 100, 167);
      hp_slider.pos = {pos_.x + 163, pos_.y + 63};
      components_.push_back(hp_slider);
      Slider mp_slider(hp_warn_, 100, 167);
      mp_slider.pos = {pos_.x + 163, pos_.y + 95};
      components_.push_back(mp_slider);

      ComboBox enemy = enemy_info_cm_;
      enemy.rect = {pos_.x + 131, pos_.y + 124};
      components_.push_back(enemy);
      ComboBox format = screenshot_format_;
      format.rect = {pos_.x + 131, pos_.y + 164};
      components_.push_back(format);
      ComboBox location = screenshot_location_;
      location.rect = {pos_.x + 131, pos_.y + 197};
      components_.push_back(location);

      CheckBox capture{.rect = SDL_FRect{pos_.x + 1, pos_.y + 1},
                       .checked = continue_capture_};
      components_.push_back(capture);
      CheckBox auto_capture{.rect = SDL_FRect{pos_.x + 1, pos_.y + 1},
                            .checked = auto_capture_};
      components_.push_back(auto_capture);
      CheckBox memory{.rect = SDL_FRect{pos_.x + 1, pos_.y + 1},
                      .checked = memory_map_};
      components_.push_back(memory);
      break;
    }
    case OptionType::kSocial: {
      CheckBox whisper{.rect = SDL_FRect{pos_.x + 1, pos_.y + 1},
                       .checked = whisper_};
      components_.push_back(whisper);
      break;
    }
    default: {
      break;
    }
  }
  return components_;
}


bool OptionUiSystem::Render() {
  RenderBackgrnd();
  RenderUI();
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

bool OptionUiSystem::EventUI(SDL_Event* event) {
  const static auto tab_node = WzResource::ui_->find(u"Options.img/tab");
  std::vector<SDL_FRect> rects;
  for (uint8_t i = 0; i < 5; i++) {
    rects.push_back(
        SDL_FRect{static_cast<float>(int(pos_.x + 10 + i * 71)),
                  static_cast<float>(int(pos_.y + 24)), 71, 21});
  }
  Tab tab{tab_node, u"enabled", u"disabled", rects, active_tab_};
  if (auto i = tab.Click()) {
    active_tab_ = *i;
  }
  return DispatchButtonClick(GetComponent(), false);
}

void OptionUiSystem::EventMotionBtAudio(SDL_Event* event) {
  static int8_t bt_audio = -1;
  int8_t bt_in = static_cast<int8_t>(ButtonHoverIndex(GetComponent()));
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
          r = !EventUI(event);
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
