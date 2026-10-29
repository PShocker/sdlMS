#include "src/client/system/ui/character_choose_ui_system.h"

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
#include "src/client/system/ui/component/ui_component.h"
#include "src/client/system/ui/login_notice_ui_system.h"
#include "src/client/system/ui/login_ui_system.h"
#include "src/client/system_instance/character_create_system_instance.h"
#include "src/client/system_instance/game_save_system_instance.h"
#include "src/client/system_instance/login_notice_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <uv.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "wz/Node.h"

namespace sdlms {

SDL_FPoint CharacterChooseUiSystem::LoadPos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto& camera = CameraGameInstance::camera_;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void CharacterChooseUiSystem::RenderUI() {
  RenderComponents(GetComponent());
}

UIComponents& CharacterChooseUiSystem::GetComponent() {
  components_.clear();
  const static std::array<wz::Node*, 4> buttons_nodes = {
      WzResource::ui_->find(u"Login.img/CharSelect/button:select"),
      WzResource::ui_->find(u"Login.img/CharSelect/button:new"),
      WzResource::ui_->find(u"Login.img/CharSelect/button:delete"),
      // 返回按钮画在背景图上，没有独立的 WZ 节点，仅提供点击区域
      nullptr,
  };
  auto pos = LoadPos();
  auto& camera = CameraGameInstance::camera_;
  std::array buttons_rect = {
      SDL_FRect{146 - camera.x, -656 - camera.y, 129, 41},
      SDL_FRect{146 - camera.x, -607 - camera.y, 129, 45},
      SDL_FRect{146 - camera.x, -544 - camera.y, 129, 55},
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
  };
  std::vector<Button::Callback> callbacks = {
      EventButtonSelect,
      EventButtonNew,
      EventButtonDelete,
      EventButtonBack,
  };
  std::vector<bool> enable = {
      choose_.has_value(),
      true,
      choose_.has_value(),
      true,
  };
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x = (int)rect.x;
    rect.y = (int)rect.y;
    components_.push_back(
        Button{.node = buttons_nodes[i],
               .rect = rect,
               .callback = callbacks[i],
               .enable = enable[i],
               .focused = true});
  }
  return components_;
}


void CharacterChooseUiSystem::RenderBanner() {
  static auto t = WzResource::LoadTexture(
      WzResource::ui_->find(u"Login.img/LoginStart/StepBanner/CharSelect"));
  auto pos = LoadPos();
  SDL_FRect pos_rect{
      pos.x + 160,
      pos.y + 85,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
}

void CharacterChooseUiSystem::RenderBackgrnd() {
  static auto t = WzResource::LoadTexture(
      WzResource::ui_->find(u"Login.img/Common/classicFrame"));
  auto pos = LoadPos();
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
}

void CharacterChooseUiSystem::RenderCharacter() {
  SDL_FPoint pos = LoadPos();
  const std::array character_pos = {
      SDL_FPoint{-270, -404},
      SDL_FPoint{-110, -404},
      SDL_FPoint{50, -404},
  };
  for (uint8_t i = 0; i < characters_.size(); i++) {
    auto& character = characters_[i];
    if (choose_.has_value() && choose_ == i) {
      RenderCharacterEffect(i);
      RenderCharacterBoard(i);
    }
    character.pos.x = character_pos[i].x;
    character.pos.y = character_pos[i].y;
    CharacterRenderSystem::Render(character);
    CharacterRenderSystem::RenderNametag(character);
  }
}

void CharacterChooseUiSystem::RenderCharacterBoard(uint8_t i) {
  const std::array board_pos = {
      SDL_FPoint{-366, -684},
      SDL_FPoint{-206, -684},
      SDL_FPoint{-46, -684},
  };
  auto& ani = board_;
  const static std::vector<SDL_Texture*> textures = {
      WzResource::LoadTexture(
          WzResource::ui_->find(u"Login.img/CharSelect/Statboard/open/0")),
      WzResource::LoadTexture(
          WzResource::ui_->find(u"Login.img/CharSelect/Statboard/open/1")),
      WzResource::LoadTexture(
          WzResource::ui_->find(u"Login.img/CharSelect/Statboard/open/2")),
      WzResource::LoadTexture(
          WzResource::ui_->find(u"Login.img/CharSelect/Statboard/open/3")),
  };
  auto& camera = CameraGameInstance::camera_;
  auto t = textures.at(ani.ani_index);
  SDL_FRect pos_rect{
      board_pos[i].x - camera.x,
      board_pos[i].y - camera.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
  if (ani.ani_index == ani.ani_delay.size() - 1) {
    // render stat
    static SDL_Texture* char_info =
        WzResource::LoadTexture(WzResource::ui_->find(
            u"Login.img/CharSelect/Statboard/canvas:charInfo"));
    pos_rect.x += 31;
    pos_rect.y += 35;
    pos_rect.w = char_info->w;
    pos_rect.h = char_info->h;
    SDL_RenderTexture(Window::renderer_, char_info, nullptr, &pos_rect);

    const auto& cs = GameSaveSystemInstance::save_.characters[i];
    Freetype::LoadSize(13);
    Freetype::LoadAligned(true);
    Freetype::LoadColor(0, 0, 0, 255);
    auto str1 = std::to_string(cs.ap.str_ap);
    auto str2 = std::u16string{str1.begin(), str1.end()};
    Freetype::DrawLine(str2, 0, 0);

    str1 = std::to_string(cs.ap.dex_ap);
    str2 = std::u16string{str1.begin(), str1.end()};
    Freetype::DrawLine(str2, 0, 0);

    str1 = std::to_string(cs.ap.int_ap);
    str2 = std::u16string{str1.begin(), str1.end()};
    Freetype::DrawLine(str2, 0, 0);

    str1 = std::to_string(cs.ap.luk_ap);
    str2 = std::u16string{str1.begin(), str1.end()};
    Freetype::DrawLine(str2, 0, 0);
    Freetype::LoadAligned(false);
  }
}

void CharacterChooseUiSystem::RenderCharacterEffect(uint8_t i) {
  const std::array effect_pos = {
      SDL_FPoint{-270, -680},
      SDL_FPoint{-110, -680},
      SDL_FPoint{50, -680},
  };
  const static auto effect_node =
      WzResource::ui_->find(u"Login.img/CharSelect/animation:selectEffect1");
  auto& camera = CameraGameInstance::camera_;
  auto texture_node = effect_node->get_child(std::to_string(effect_.ani_index));
  auto origin = WzResource::LoadFpoint(texture_node->get_child(u"origin"));
  auto t = WzResource::LoadTexture(texture_node);

  SDL_FRect pos_rect{
      effect_pos[i].x - camera.x - origin.x,
      effect_pos[i].y - camera.y - origin.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
}

bool CharacterChooseUiSystem::Render() {
  RenderUI();
  RenderCharacter();
  RenderBackgrnd();
  RenderBanner();
  return true;
}

void CharacterChooseUiSystem::EventButtonSelect() {
  if (!choose_.has_value()) {
    return;
  }
  AudioGameInstance::LoadAudio(u"Game.img/GameIn", 0);

  auto cse = choose_.value();
  auto character = characters_[cse];
  uint32_t map_id;
  uint32_t portal_id;
  // load
  for (auto& cs : GameSaveSystemInstance::save_.characters) {
    if (cs.character.name == character.name) {
      map_id = cs.map_id;
      portal_id = cs.portal_id;
      // load pack,ski,ap,sp
      PackageGameInstance::Load(cs);
      CharacterStatGameInstance::Load(cs);
      JobSkillGameInstance::Load(cs);
      KeyboardGameInstance::Load(cs);
    }
  }
  // init
  CharacterGameInstance::self_ = character;

  SceneSystemInstance::EnterPrepare(map_id, u"sp", portal_id);
  // notice login
  auto NoticeType = LoginNoticeSystemInstance::NoticeType::kLogining;
  LoginNoticeSystemInstance::Enter(NoticeType, NULL);
  return;
}

void CharacterChooseUiSystem::EventButtonNew() {
  if (characters_.size() >= 3) {
    LoginNoticeSystemInstance::Enter(
        LoginNoticeSystemInstance::NoticeType::kCharacterFull, nullptr);
    AudioGameInstance::LoadAudio(u"UI.img/DlgNotice", 0);
    return;
  }
  AudioGameInstance::LoadAudio(u"UI.img/ScrollUp", 0);

  CharacterCreateSystemInstance::EnterPrepare();

  System::logic_systems_.push_back(NewAnimate);
  System::render_systems_ = {
      LoginSystemInstance::RenderGame,
      CharacterCreateUiSystem::Render,
      CursorRenderSystem::Render,
  };
  System::event_systems_ = {};
}

void CharacterChooseUiSystem::EventButtonDeleteCb() {
  if (!choose_.has_value()) {
    return;
  }
  auto cse = choose_.value();
  auto character = characters_[cse];
  auto& save = GameSaveSystemInstance::save_;
  for (int i = 0; i < save.characters.size(); i++) {
    auto& save_character = save.characters[i].character;
    if (save_character.name == character.name) {
      save.characters.erase(save.characters.begin() + i);
    }
  }
  characters_.erase(characters_.begin() + cse);
  choose_ = std::nullopt;
  LoginNoticeUiSystem::EventClose();
}

void CharacterChooseUiSystem::EventButtonDelete() {
  AudioGameInstance::LoadAudio(u"UI.img/DlgNotice", 0);
  LoginNoticeSystemInstance::Enter(
      LoginNoticeSystemInstance::NoticeType::kCharacterDelete,
      EventButtonDeleteCb);
  return;
}

bool CharacterChooseUiSystem::NewAnimate() {
  if (LoginUiSystem::CameraAnimate(-80, -1294)) {
    CharacterCreateSystemInstance::Enter();
    return false;
  }
  return true;
}

bool CharacterChooseUiSystem::BackAnimate() {
  if (LoginUiSystem::CameraAnimate(-80, 1023)) {
    LoginSystemInstance::Enter();
    return false;
  }
  auto& camera = CameraGameInstance::camera_;
  LoginUiSystem::username_.r.x = (-30 - camera.x);
  LoginUiSystem::username_.r.y = (960 - camera.y);
  return true;
}

void CharacterChooseUiSystem::EventButtonBack() {
  AudioGameInstance::LoadAudio(u"UI.img/ScrollUp", 0);

  GameSaveSystemInstance::SaveGame();
  LoginSystemInstance::EnterPrepare();

  System::logic_systems_.push_back(BackAnimate);
  System::render_systems_ = {
      LoginSystemInstance::RenderGame,
      LoginUiSystem::Render,
      CursorRenderSystem::Render,
  };
  System::event_systems_ = {};
}

bool CharacterChooseUiSystem::EventChooseCharacter(SDL_Event* event) {
  std::optional<uint8_t> cse;
  const std::array character_pos = {
      SDL_FPoint{-270, -404},
      SDL_FPoint{-110, -404},
      SDL_FPoint{50, -404},
  };
  auto& camera = CameraGameInstance::camera_;
  for (int i = 0; i < characters_.size(); i++) {
    auto pos = character_pos[i];
    auto character = characters_[i];
    character.pos = pos;
    auto r = CharacterLogicSystem::LoadRect(character);
    r.x -= camera.x;
    r.y -= camera.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &r)) {
      cse = i;
      break;
    }
  }
  if (cse == std::nullopt) {
    return false;
  }
  if (cse == choose_) {
    EventButtonSelect();
  } else {
    choose_ = cse;
    for (auto& character : characters_) {
      CharacterLogicSystem::RunStandAction(character);
    }
    CharacterLogicSystem::RunWalkAction(characters_[cse.value()]);
    board_.ani_index = 0;
    board_.ani_time = 0;
    effect_.ani_index = 0;
    effect_.ani_time = 0;
    AudioGameInstance::LoadAudio(u"UI.img/CharSelect", 0);
  }
  return true;
}

bool CharacterChooseUiSystem::EventUI(SDL_Event* event) {
  if (EventChooseCharacter(event)) {
    return true;
  }
  return DispatchButtonClick(GetComponent(), false);
}

bool CharacterChooseUiSystem::Event(SDL_Event* event) {
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
        r = !EventUI(event);
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

bool CharacterChooseUiSystem::Run() {
  for (auto& character : characters_) {
    CharacterLogicSystem::RunAnimate(character);
  }
  if (choose_.has_value()) {
    auto anis = {&board_, &effect_};
    for (auto ani : anis) {
      if (ani->ani_index < ani->ani_delay.size()) {
        ani->ani_time += Window::delta_time_;
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

}  // namespace sdlms
