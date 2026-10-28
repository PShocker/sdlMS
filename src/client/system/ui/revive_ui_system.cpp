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

void ReviveUiSystem::render_backgrnd() {
  static auto texture =
      WzResource::load_texture(WzResource::ui->find(u"Revive.img/backgrnd"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
}

void ReviveUiSystem::render_button() {
  const static std::array buttons_node = {
      WzResource::ui->find(u"Basic.img/BtOK2"),
  };
  const std::array buttons_rect = {
      SDL_FRect{15, 84, 47, 18}, //
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

bool ReviveUiSystem::render() {
  render_backgrnd();
  render_button();
  return true;
}

SDL_FPoint ReviveUiSystem::load_wh() { return {286, 146}; }

void ReviveUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void ReviveUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
}

void ReviveUiSystem::toggle() {
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool ReviveUiSystem::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void ReviveUiSystem::event_motion_bt_audio(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  const std::array buttons_rect = {
      SDL_FRect{15, 84, 47, 18}, //
  };
  static int8_t bt_audio = -1;
  auto &mouse_pos = Window::mouse_pos;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      bt_in = i;
      break;
    }
  }
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::load_audio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool ReviveUiSystem::event(SDL_Event *event) {
  KeyboardInputSystem::event(event);
  bool r = false;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        r = !event_button(event);
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_motion_bt_audio(event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

void ReviveUiSystem::event_button_ok() {
  auto map_id = SceneSystemInstance::map_id;
  auto r_map = MapInfoGameInstance::load_return_map(map_id);

  SceneSystemInstance::enter_prepare(r_map, u"sp", 0);

  CharacterLogicSystem::self_portal_cooldown = Window::dt_now + 1500;

  auto &v = SkillGameInstance::ski;
  std::vector<std::function<void()>> ends;
  for (const auto &sk : v) {
    ends.push_back(sk.end);
  }
  for (auto fn : ends) {
    fn();
  }
  auto hp_max = CharacterStatGameInstance::get_hp_max();
  auto mp_max = CharacterStatGameInstance::get_mp_max();
  CharacterStatGameInstance::set_hp(hp_max);
  CharacterStatGameInstance::set_mp(mp_max);
  close();
  return;
}

bool ReviveUiSystem::event_button(SDL_Event *event) {
  const std::array buttons_rect = {
      SDL_FRect{15, 84, 47, 18}, //
  };
  const static std::array buttons_func = {
      event_button_ok,
  };

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      buttons_func[i]();
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }
  return false;
}