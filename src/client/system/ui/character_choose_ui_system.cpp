#include "character_choose_ui_system.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "login_notice_ui_system.h"
#include "login_ui_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_create_ui_system.h"
#include "src/client/system_instance/character_create_system_instance.h"
#include "src/client/system_instance/game_save_system_instance.h"
#include "src/client/system_instance/login_notice_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "uv.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

SDL_FPoint CharacterChooseUiSystem::load_pos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto &camera = CameraGameInstance::camera;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void CharacterChooseUiSystem::render_button() {
  const static std::array buttons_nodes = {
      WzResource::ui->find(u"Login.img/CharSelect/button:select"),
      WzResource::ui->find(u"Login.img/CharSelect/button:new"),
      WzResource::ui->find(u"Login.img/CharSelect/button:delete"),
  };
  auto pos = load_pos();
  auto &camera = CameraGameInstance::camera;
  std::array buttons_rect = {
      SDL_FRect{146 - camera.x, -656 - camera.y, 129, 41},
      SDL_FRect{146 - camera.x, -607 - camera.y, 129, 45},
      SDL_FRect{146 - camera.x, -544 - camera.y, 129, 55},
  };
  std::vector<bool> disable = {
      !choose.has_value(),
      false,
      !choose.has_value(),
  };
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos;
    if (disable[i]) {
      auto d = WzResource::load_texture(k->find(u"disabled/0"));
      SDL_RenderTexture(Window::renderer, d, nullptr, &pos_rect);
    } else if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
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

void CharacterChooseUiSystem::render_banner() {
  static auto t = WzResource::load_texture(
      WzResource::ui->find(u"Login.img/LoginStart/StepBanner/CharSelect"));
  auto pos = load_pos();
  SDL_FRect pos_rect{
      pos.x + 160,
      pos.y + 85,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);

  const static std::array buttons_nodes = {
      WzResource::ui->find(u"Login.img/LoginStart/BtClassicPrev"),
  };
  std::array buttons_rect = {
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
  };
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
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

void CharacterChooseUiSystem::render_backgrnd() {
  static auto t = WzResource::load_texture(
      WzResource::ui->find(u"Login.img/Common/classicFrame"));
  auto pos = load_pos();
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
}

void CharacterChooseUiSystem::render_character() {
  SDL_FPoint pos = load_pos();
  const std::array character_pos = {
      SDL_FPoint{-270, -404},
      SDL_FPoint{-110, -404},
      SDL_FPoint{50, -404},
  };
  for (uint8_t i = 0; i < characters.size(); i++) {
    auto &character = characters[i];
    if (choose.has_value() && choose == i) {
      render_character_effect(i);
      render_character_board(i);
    }
    character.pos.x = character_pos[i].x;
    character.pos.y = character_pos[i].y;
    CharacterRenderSystem::render(character);
    CharacterRenderSystem::render_nametag(character);
  }
}

void CharacterChooseUiSystem::render_character_board(uint8_t i) {
  const std::array board_pos = {
      SDL_FPoint{-366, -684},
      SDL_FPoint{-206, -684},
      SDL_FPoint{-46, -684},
  };
  auto &ani = board;
  const static std::vector<SDL_Texture *> textures = {
      WzResource::load_texture(
          WzResource::ui->find(u"Login.img/CharSelect/Statboard/open/0")),
      WzResource::load_texture(
          WzResource::ui->find(u"Login.img/CharSelect/Statboard/open/1")),
      WzResource::load_texture(
          WzResource::ui->find(u"Login.img/CharSelect/Statboard/open/2")),
      WzResource::load_texture(
          WzResource::ui->find(u"Login.img/CharSelect/Statboard/open/3")),
  };
  auto &camera = CameraGameInstance::camera;
  auto t = textures.at(ani.ani_index);
  SDL_FRect pos_rect{
      board_pos[i].x - camera.x,
      board_pos[i].y - camera.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  if (ani.ani_index == ani.ani_delay.size() - 1) {
    // render stat
    static SDL_Texture *charInfo =
        WzResource::load_texture(WzResource::ui->find(
            u"Login.img/CharSelect/Statboard/canvas:charInfo"));
    pos_rect.x += 31;
    pos_rect.y += 35;
    pos_rect.w = charInfo->w;
    pos_rect.h = charInfo->h;
    SDL_RenderTexture(Window::renderer, charInfo, nullptr, &pos_rect);

    const auto &cs = GameSaveSystemInstance::save.characters[i];
    Freetype::load_size(13);
    Freetype::load_aligned(true);
    Freetype::load_color(0, 0, 0, 255);
    auto str1 = std::to_string(cs.ap.str_ap);
    auto str2 = std::u16string{str1.begin(), str1.end()};
    Freetype::draw_line(str2, 0, 0);

    str1 = std::to_string(cs.ap.dex_ap);
    str2 = std::u16string{str1.begin(), str1.end()};
    Freetype::draw_line(str2, 0, 0);

    str1 = std::to_string(cs.ap.int_ap);
    str2 = std::u16string{str1.begin(), str1.end()};
    Freetype::draw_line(str2, 0, 0);

    str1 = std::to_string(cs.ap.luk_ap);
    str2 = std::u16string{str1.begin(), str1.end()};
    Freetype::draw_line(str2, 0, 0);
    Freetype::load_aligned(false);
  }
}

void CharacterChooseUiSystem::render_character_effect(uint8_t i) {
  const std::array effect_pos = {
      SDL_FPoint{-270, -680},
      SDL_FPoint{-110, -680},
      SDL_FPoint{50, -680},
  };
  const static auto effect_node =
      WzResource::ui->find(u"Login.img/CharSelect/animation:selectEffect1");
  auto &camera = CameraGameInstance::camera;
  auto texture_node = effect_node->get_child(std::to_string(effect.ani_index));
  auto origin = WzResource::load_fpoint(texture_node->get_child(u"origin"));
  auto t = WzResource::load_texture(texture_node);

  SDL_FRect pos_rect{
      effect_pos[i].x - camera.x - origin.x,
      effect_pos[i].y - camera.y - origin.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
}

bool CharacterChooseUiSystem::render() {
  render_button();
  render_character();
  render_backgrnd();
  render_banner();
  return true;
}

void CharacterChooseUiSystem::event_button_select() {
  if (!choose.has_value()) {
    return;
  }
  AudioGameInstance::load_audio(u"Game.img/GameIn", 0);

  auto cse = choose.value();
  auto character = characters[cse];
  uint32_t map_id;
  uint32_t portal_id;
  // load
  for (auto &cs : GameSaveSystemInstance::save.characters) {
    if (cs.character.name == character.name) {
      map_id = cs.map_id;
      portal_id = cs.portal_id;
      // load pack,ski,ap,sp
      PackageGameInstance::load(cs);
      CharacterStatGameInstance::load(cs);
      JobSkillGameInstance::load(cs);
      KeyboardGameInstance::load(cs);
    }
  }
  // init
  CharacterGameInstance::self = character;

  SceneSystemInstance::enter_prepare(map_id, u"sp", portal_id);
  // notice login
  auto NoticeType = LoginNoticeSystemInstance::logining;
  LoginNoticeSystemInstance::enter(NoticeType, NULL);
  return;
}

void CharacterChooseUiSystem::event_button_new() {
  if (characters.size() >= 3) {
    LoginNoticeSystemInstance::enter(
        LoginNoticeSystemInstance::character_full, nullptr);
    AudioGameInstance::load_audio(u"UI.img/DlgNotice", 0);
    return;
  }
  AudioGameInstance::load_audio(u"UI.img/ScrollUp", 0);

  CharacterCreateSystemInstance::enter_prepare();

  System::logic_systems.push_back(new_animate);
  System::render_systems = {
      LoginSystemInstance::render_game,
      CharacterCreateUiSystem::render,
      CursorRenderSystem::render,
  };
  System::event_systems = {};
}

void CharacterChooseUiSystem::event_button_delete_cb() {
  if (!choose.has_value()) {
    return;
  }
  auto cse = choose.value();
  auto character = characters[cse];
  auto &save = GameSaveSystemInstance::save;
  for (int i = 0; i < save.characters.size(); i++) {
    auto &save_character = save.characters[i].character;
    if (save_character.name == character.name) {
      save.characters.erase(save.characters.begin() + i);
    }
  }
  characters.erase(characters.begin() + cse);
  choose = std::nullopt;
  LoginNoticeUiSystem::event_close();
}

void CharacterChooseUiSystem::event_button_delete() {
  AudioGameInstance::load_audio(u"UI.img/DlgNotice", 0);
  LoginNoticeSystemInstance::enter(
      LoginNoticeSystemInstance::character_delete, event_button_delete_cb);
  return;
}

bool CharacterChooseUiSystem::new_animate() {
  if (LoginUiSystem::camera_animate(-80, -1294)) {
    CharacterCreateSystemInstance::enter();
    return false;
  }
  return true;
}

bool CharacterChooseUiSystem::back_animate() {
  if (LoginUiSystem::camera_animate(-80, 1023)) {
    LoginSystemInstance::enter();
    return false;
  }
  auto &camera = CameraGameInstance::camera;
  LoginUiSystem::username.r.x = (-30 - camera.x);
  LoginUiSystem::username.r.y = (960 - camera.y);
  return true;
}

void CharacterChooseUiSystem::event_button_back() {
  AudioGameInstance::load_audio(u"UI.img/ScrollUp", 0);

  GameSaveSystemInstance::save_game();
  LoginSystemInstance::enter_prepare();

  System::logic_systems.push_back(back_animate);
  System::render_systems = {
      LoginSystemInstance::render_game,
      LoginUiSystem::render,
      CursorRenderSystem::render,
  };
  System::event_systems = {};
}

bool CharacterChooseUiSystem::event_choose_character(SDL_Event *event) {
  std::optional<uint8_t> cse;
  const std::array character_pos = {
      SDL_FPoint{-270, -404},
      SDL_FPoint{-110, -404},
      SDL_FPoint{50, -404},
  };
  auto &camera = CameraGameInstance::camera;
  for (int i = 0; i < characters.size(); i++) {
    auto pos = character_pos[i];
    auto character = characters[i];
    character.pos = pos;
    auto r = CharacterLogicSystem::load_rect(character);
    r.x -= camera.x;
    r.y -= camera.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &r)) {
      cse = i;
      break;
    }
  }
  if (cse == std::nullopt) {
    return false;
  }
  if (cse == choose) {
    event_button_select();
  } else {
    choose = cse;
    for (auto &character : characters) {
      CharacterLogicSystem::run_stand_action(character);
    }
    CharacterLogicSystem::run_walk_action(characters[cse.value()]);
    board.ani_index = 0;
    board.ani_time = 0;
    effect.ani_index = 0;
    effect.ani_time = 0;
    AudioGameInstance::load_audio(u"UI.img/CharSelect", 0);
  }
  return true;
}

bool CharacterChooseUiSystem::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto &camera = CameraGameInstance::camera;
  auto pos = load_pos();
  r = {
      SDL_FRect{146 - camera.x, -656 - camera.y, 129, 41},
      SDL_FRect{146 - camera.x, -607 - camera.y, 129, 45},
      SDL_FRect{146 - camera.x, -544 - camera.y, 129, 55},
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
  };
  fns = {
      event_button_select,
      event_button_new,
      event_button_delete,
      event_button_back,
  };
  std::vector<bool> disable = {
      !choose.has_value(),
      false,
      !choose.has_value(),
      false,
  };
  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect) && !disable[i]) {
      fns[i]();
      return false;
    }
  }

  return true;
}

bool CharacterChooseUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      r = false;
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      event_choose_character(event);
      r = event_button(event);
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

bool CharacterChooseUiSystem::run() {
  for (auto &character : characters) {
    CharacterLogicSystem::run_animate(character);
  }
  if (choose.has_value()) {
    auto anis = {&board, &effect};
    for (auto ani : anis) {
      if (ani->ani_index < ani->ani_delay.size()) {
        ani->ani_time += Window::delta_time;
        auto delay = ani->ani_delay[ani->ani_index];
        if (delay == 0) {
          continue;
        }
        if (ani->ani_time >= delay) {
          ani->ani_index++;
          ani->ani_time = 0;
        }
      }
    }
  }
  return true;
}