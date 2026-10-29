#include "src/client/system/ui/revive_ui_system.h"

#include "src/client/system/ui/component/button.h"
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

namespace sdlms {

void ReviveUiSystem::RenderBackgrnd() {
  static auto texture =
      WzResource::LoadTexture(WzResource::ui_->find(u"Revive.img/backgrnd"));
  SDL_FRect pos_rect{pos_.x, pos_.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

void ReviveUiSystem::RenderButton() {
  components_.clear();
  const static std::array buttons_node = {
      WzResource::ui_->find(u"Basic.img/BtOK2"),
  };
  const std::array buttons_rect = {
      SDL_FRect{15, 84, 47, 18},  //
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x += pos_.x;
    rect.y += pos_.y;
    Button button{.node = buttons_node[i],
                  .rect = rect,
                  .enable = true,
                  .focused = CursorGameInstance::cursor_ui_ == Render &&
                             !CursorGameInstance::modal_overlay_};
    components_.push_back(button);
    button.Render();
  }
}

std::vector<Button>& ReviveUiSystem::GetComponent() {
  return components_;
}


bool ReviveUiSystem::Render() {
  RenderBackgrnd();
  RenderButton();
  return true;
}

SDL_FPoint ReviveUiSystem::LoadWh() {
  return {286, 146};
}

void ReviveUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
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
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void ReviveUiSystem::EventMotionBtAudio(SDL_Event* event) {
  if (CursorGameInstance::cursor_ui_ != Render) {
    return;
  }
  const std::array buttons_rect = {
      SDL_FRect{15, 84, 47, 18},  //
  };
  static int8_t bt_audio = -1;
  auto& mouse_pos = Window::mouse_pos_;
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

bool ReviveUiSystem::Event(SDL_Event* event) {
  KeyboardInputSystem::Event(event);
  bool r = false;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
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
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventMotionBtAudio(event);
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

  auto& v = SkillGameInstance::ski_;
  std::vector<std::function<void()>> ends;
  for (const auto& sk : v) {
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

bool ReviveUiSystem::EventButton(SDL_Event* event) {
  const std::array buttons_rect = {
      SDL_FRect{15, 84, 47, 18},  //
  };
  const static std::array buttons_func = {
      EventButtonOk,
  };

  for (size_t i = 0; i < components_.size() && i < buttons_func.size(); ++i) {
    components_[i].callback = buttons_func[i];
  }
  if (DispatchButtonClick(components_, true)) {
    return true;
  }
  return false;
}

}  // namespace sdlms
