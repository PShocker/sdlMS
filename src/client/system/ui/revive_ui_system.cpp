#include "revive_ui_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <optional>

void ReviveUiSystem::RenderBackgrnd() {
  static auto texture =
      WzResource::LoadTexture(WzResource::ui_->find(u"Revive.img/backgrnd"));
  SDL_FRect pos_rect{pos_.x, pos_.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

void ReviveUiSystem::RenderButton() {
  const static std::array buttons_node = {
      WzResource::ui_->find(u"Basic.img/BtOK2"),
  };
  const std::array buttons_rect = {
      SDL_FRect{15, 84, 47, 18}, //
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    auto &mouse_pos = Window::mouse_pos_;
    // 判断按钮是否被遮挡
    auto CursorIn = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && CursorIn == Render &&
        !CursorGameInstance::modal_overlay) {
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

bool ReviveUiSystem::Render() {
  RenderBackgrnd();
  RenderButton();
  return true;
}

SDL_FPoint ReviveUiSystem::LoadWh() { return {286, 146}; }

void ReviveUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto &camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void ReviveUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void ReviveUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool ReviveUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto &mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void ReviveUiSystem::EventMotionBtAudio(SDL_Event *Event) {
  if (CursorGameInstance::cursor_ui != Render) {
    return;
  }
  const std::array buttons_rect = {
      SDL_FRect{15, 84, 47, 18}, //
  };
  static int8_t bt_audio = -1;
  auto &mouse_pos = Window::mouse_pos_;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos_.x;
    pos_rect.y += (int)pos_.y;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      bt_in = i;
      break;
    }
  }
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool ReviveUiSystem::Event(SDL_Event *Event) {
  KeyboardInputSystem::Event(Event);
  bool r = false;
  switch (Event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        r = !EventButton(Event);
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    EventMotionBtAudio(Event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

void ReviveUiSystem::EventButtonOk() {
  auto map_id = SceneSystemInstance::map_id_;
  auto r_map = MapInfoGameInstance::LoadReturnMap(map_id);

  SceneSystemInstance::EnterPrepare(r_map, u"sp", 0);

  CharacterLogicSystem::self_portal_cooldown_ = Window::dt_now_ + 1500;

  auto &v = SkillGameInstance::ski_;
  std::vector<std::function<void()>> ends;
  for (const auto &sk : v) {
    ends.push_back(sk.end);
  }
  for (auto fn : ends) {
    fn();
  }
  auto hp_max = CharacterStatGameInstance::GetHpMax();
  auto mp_max = CharacterStatGameInstance::GetMpMax();
  CharacterStatGameInstance::SetHp(hp_max);
  CharacterStatGameInstance::SetMp(mp_max);
  Close();
  return;
}

bool ReviveUiSystem::EventButton(SDL_Event *Event) {
  const std::array buttons_rect = {
      SDL_FRect{15, 84, 47, 18}, //
  };
  const static std::array buttons_func = {
      EventButtonOk,
  };

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      buttons_func[i]();
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }
  return false;
}