#include "option_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "slider_ui_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/combo_box_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include <string>
#include <vector>

enum class OptionType {
  kGraphic,
  kSound,
  kGame,
  kSocial,
  kController,
};

void OptionUiSystem::RenderBackgrnd() {
  static auto texture =
      WzResource::LoadTexture(WzResource::ui->find(u"Options.img/backgrnd"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
}

void OptionUiSystem::RenderOption() {
  std::vector<wz::Node *> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  auto top = CursorGameInstance::cursor_ui == Render;

  switch (static_cast<OptionType>(active_tab)) {
  case OptionType::kGraphic: {
    static auto backgrnd = WzResource::LoadTexture(
        WzResource::ui->find(u"Options.img/graphic/layer:backgrnd"));
    SDL_FRect pos_rect{
        pos.x + 9,
        pos.y + 53,
        static_cast<float>(backgrnd->w),
        static_cast<float>(backgrnd->h),
    };
    SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);
    break;
  }
  case OptionType::kSound: {
    static auto backgrnd = WzResource::LoadTexture(
        WzResource::ui->find(u"Options.img/sound/layer:backgrnd"));
    SDL_FRect pos_rect{
        pos.x + 9,
        pos.y + 53,
        static_cast<float>(backgrnd->w),
        static_cast<float>(backgrnd->h),
    };
    SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);
    // master volume
    // hp warn
    Freetype::LoadAligned(true);
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawCstr(u"Master Volume", pos.x + 12, pos.y + 53, 105, 1);
    Freetype::LoadColor(0, 0, 0, 255);

    Freetype::DrawLine(u"0%", pos.x + 136, pos.y + 91);
    SliderUiSystem::RenderHslider(pos.x + 163, pos.y + 95, hp_warn, 100, 167,
                                     top, false);
    Freetype::DrawLine(u"100%", pos.x + 335, pos.y + 91);
    SDL_Texture *t;
    if (master_volume_mute) {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    Freetype::LoadAligned(true);
    Freetype::LoadSize(12);
    Freetype::LoadColor(0, 0, 0, 255);
    Freetype::DrawLine(u"Mute", pos.x, pos.y);

    // BGM
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawCstr(u"BGM", pos.x + 12, pos.y + 53, 105, 1);
    Freetype::LoadColor(0, 0, 0, 255);

    Freetype::DrawLine(u"0%", pos.x + 136, pos.y + 91);
    SliderUiSystem::RenderHslider(pos.x + 163, pos.y + 95, hp_warn, 100, 167,
                                     top, false);
    Freetype::DrawLine(u"100%", pos.x + 335, pos.y + 91);
    if (bgm_volume_mute) {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    Freetype::LoadAligned(true);
    Freetype::LoadSize(12);
    Freetype::LoadColor(0, 0, 0, 255);
    Freetype::DrawLine(u"Mute", pos.x, pos.y);

    // Normal SFX
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawCstr(u"Normal SFX", pos.x + 12, pos.y + 53, 105, 1);
    Freetype::LoadColor(0, 0, 0, 255);

    Freetype::DrawLine(u"0%", pos.x + 136, pos.y + 91);
    SliderUiSystem::RenderHslider(pos.x + 163, pos.y + 95, hp_warn, 100, 167,
                                     top, false);
    Freetype::DrawLine(u"100%", pos.x + 335, pos.y + 91);
    if (normal_volume_mute) {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    Freetype::LoadAligned(true);
    Freetype::LoadSize(12);
    Freetype::LoadColor(0, 0, 0, 255);
    Freetype::DrawLine(u"Mute", pos.x, pos.y);

    // Monster SFX
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawCstr(u"Monster SFX", pos.x + 12, pos.y + 53, 105, 1);
    Freetype::LoadColor(0, 0, 0, 255);

    Freetype::DrawLine(u"0%", pos.x + 136, pos.y + 91);
    SliderUiSystem::RenderHslider(pos.x + 163, pos.y + 95, hp_warn, 100, 167,
                                     top, false);
    Freetype::DrawLine(u"100%", pos.x + 335, pos.y + 91);
    if (monster_volume_mute) {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    Freetype::LoadAligned(true);
    Freetype::LoadSize(12);
    Freetype::LoadColor(0, 0, 0, 255);
    Freetype::DrawLine(u"Mute", pos.x, pos.y);

    // Skill SFX
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawCstr(u"Skill SFX", pos.x + 12, pos.y + 53, 105, 1);
    Freetype::LoadColor(0, 0, 0, 255);

    Freetype::DrawLine(u"0%", pos.x + 136, pos.y + 91);
    SliderUiSystem::RenderHslider(pos.x + 163, pos.y + 95, hp_warn, 100, 167,
                                     top, false);
    Freetype::DrawLine(u"100%", pos.x + 335, pos.y + 91);
    if (skill_volume_mute) {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    Freetype::LoadAligned(true);
    Freetype::LoadSize(12);
    Freetype::LoadColor(0, 0, 0, 255);
    Freetype::DrawLine(u"Mute", pos.x, pos.y);
    break;
  }
  case OptionType::kGame: {
    static auto backgrnd = WzResource::LoadTexture(
        WzResource::ui->find(u"Options.img/game/layer:backgrnd"));
    SDL_FRect pos_rect{
        pos.x + 9,
        pos.y + 53,
        static_cast<float>(backgrnd->w),
        static_cast<float>(backgrnd->h),
    };
    SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);
    // hp warn
    Freetype::LoadAligned(true);
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawCstr(u"HP Warning", pos.x + 12, pos.y + 53, 105, 1.3);
    Freetype::LoadColor(0, 0, 0, 255);

    Freetype::DrawLine(u"0%", pos.x + 136, pos.y + 58);
    // hp slider
    SliderUiSystem::RenderHslider(pos.x + 163, pos.y + 63, hp_warn, 100, 167,
                                     top, false);
    Freetype::DrawLine(u"100%", pos.x + 335, pos.y + 58);

    // mp warn
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawCstr(u"MP Warning", pos.x + 12, pos.y + 86, 105, 1.3);
    Freetype::LoadColor(0, 0, 0, 255);
    Freetype::DrawLine(u"0%", pos.x + 136, pos.y + 91);
    SliderUiSystem::RenderHslider(pos.x + 163, pos.y + 95, hp_warn, 100, 167,
                                     top, false);
    Freetype::DrawLine(u"100%", pos.x + 335, pos.y + 91);

    // enemy info
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawCstr(u"Enemy Info", pos.x + 12, pos.y + 119, 105, 1.3);
    Freetype::LoadColor(0, 0, 0, 255);
    ComboBoxUiSystem::Render(pos.x + 131, pos.y + 124, enemy_info_cm, top,
                                false);
    // screen format
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawCstr(u"ScreenShot File\n Format", pos.x + 12, pos.y + 158,
                        105, 1);
    Freetype::LoadColor(0, 0, 0, 255);
    ComboBoxUiSystem::Render(pos.x + 131, pos.y + 164, screenshot_format,
                                top, false);
    // screen folder
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawCstr(u"ScreenShot File\n Format", pos.x + 12, pos.y + 158,
                        105, 1);
    Freetype::LoadColor(0, 0, 0, 255);

    ComboBoxUiSystem::Render(pos.x + 131, pos.y + 197, screenshot_location,
                                top, false);
    // burst screen shot
    SDL_Texture *t;
    if (continue_capture) {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    std::u16string text = u"Continuous Capture";
    Freetype::LoadSize(12);
    Freetype::DrawLine(text, pos_rect.x + 100, pos_rect.y);

    auto tmp = std::to_string(continue_capture_num);
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
    Freetype::DrawLine(text, pos.x, pos.y);

    if (auto_capture) {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    text = u"Auto-Capture Memorable Moments";

    if (memory_map) {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    text = u"Use";

    tmp = std::to_string(memory_limit);
    text = std::u16string{tmp.begin(), tmp.end()} + u"   Shots";
    Freetype::LoadColor(0, 0, 0, 255);
    Freetype::DrawLine(text, pos_rect.x + 100, pos_rect.y);

    break;
  }
  case OptionType::kSocial: {
    static auto backgrnd = WzResource::LoadTexture(
        WzResource::ui->find(u"Options.img/game/layer:backgrnd"));
    SDL_FRect pos_rect{
        pos.x + 9,
        pos.y + 53,
        static_cast<float>(backgrnd->w),
        static_cast<float>(backgrnd->h),
    };
    SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);

    SDL_Texture *t;
    //whisper
    if (whisper) {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/1"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/0"));
    }
    pos_rect.x = pos.x + 1;
    pos_rect.y = pos.y + 1;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    std::u16string text = u"Continuous Capture";
    Freetype::LoadSize(12);
    Freetype::DrawLine(text, pos_rect.x + 100, pos_rect.y);
  }
  }
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto CursorIn = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && CursorIn == Render &&
        !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::LoadTexture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::LoadTexture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::LoadTexture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void OptionUiSystem::RenderButton() {
  const static std::array buttons_node = {
      WzResource::ui->find(u"KeyBindings.img/quickslot/button:confirm"),
      WzResource::ui->find(u"ChannelChange.img/Channel/BtCancel"),
      WzResource::ui->find(u"Basic.img/BtClose"),
  };
  const std::array buttons_rect = {
      SDL_FRect{176, 407, 53, 18}, //
      SDL_FRect{232, 407, 47, 18}, //
      SDL_FRect{266, 7, 12, 12},   //
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto CursorIn = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && CursorIn == Render &&
        !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::LoadTexture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::LoadTexture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::LoadTexture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void OptionUiSystem::RenderTab() {
  const static std::array tab_pos = {
      SDL_FPoint{10, 24},  //
      SDL_FPoint{81, 24},  //
      SDL_FPoint{152, 24}, //
      SDL_FPoint{223, 24}, //
      SDL_FPoint{294, 24}, //
  };
  const static auto tab_node = WzResource::ui->find(u"Options.img/tab");
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
    SDL_Texture *t = active_tab == i ? active_texture[i] : disabled_texture[i];
    SDL_FRect pos_rect{
        static_cast<float>(int(pos.x + tab_pos[i].x)),
        static_cast<float>(int(pos.y + tab_pos[i].y)),
        static_cast<float>(t->w),
        static_cast<float>(t->h),
    };
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  }
}

bool OptionUiSystem::Render() {
  RenderBackgrnd();
  RenderButton();
  RenderTab();
  RenderOption();
  return true;
}

SDL_FPoint OptionUiSystem::LoadWh() { return {287, 432}; }

void OptionUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::Render);
  if (it != System::render_systems.end()) {
    auto wh = LoadWh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    active_tab = static_cast<uint8_t>(OptionType::kGame);

    System::render_systems.insert(it, Render);
    System::event_systems.insert(System::event_systems.begin(), Event);
  }
}

void OptionUiSystem::Close() {
  std::erase(System::render_systems, Render);
  std::erase(System::event_systems, Event);
}

void OptionUiSystem::EventTop() {
  std::erase(System::render_systems, Render);
  std::erase(System::event_systems, Event);
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::Render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, Render);
    System::event_systems.insert(System::event_systems.begin(), Event);
  }
}

void OptionUiSystem::EventDragStart(SDL_Event *Event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {Event->button.x, Event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - Event->button.x, pos.y - Event->button.y};
  }
  return;
}

void OptionUiSystem::EventDragEnd() {
  drag = std::nullopt;
  return;
}

void OptionUiSystem::EventDragMove(SDL_Event *Event) {
  if (drag.has_value()) {
    pos = {Event->motion.x + drag->x, Event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = LoadWh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void OptionUiSystem::EventButtonOk() { Close(); }

void OptionUiSystem::EventTab(SDL_Event *Event) {
  const static std::array tab_rect = {
      SDL_FRect{5, 26, 33, 19},  //
      SDL_FRect{38, 26, 33, 19}, //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      active_tab = i;
    }
  }
}

bool OptionUiSystem::EventButton(SDL_Event *Event) {
  const static std::array buttons_rect = {
      SDL_FRect{119, 115, 47, 18}, //
  };
  const static std::array buttons_func = {
      EventButtonOk,
  };

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      buttons_func[i]();
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }
  return false;
}

void OptionUiSystem::EventMotionBtAudio(SDL_Event *Event) {
  if (CursorGameInstance::cursor_ui != Render) {
    return;
  }
  auto wh = LoadWh();
  std::vector<SDL_FRect> buttons_rect = {};

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
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool OptionUiSystem::Event(SDL_Event *Event) {
  bool r = true;
  switch (Event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        EventTop();
        EventDragStart(Event);
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
      EventDragEnd();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    EventDragMove(Event);
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
  if (std::ranges::contains(System::render_systems, fn)) {
    Close();
  } else {
    Open();
  }
}

bool OptionUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}