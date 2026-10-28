#include "character_create_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "login_ui_system.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_save.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_choose_ui_system.h"
#include "src/client/system/ui/text_input_ui_system.h"
#include "src/client/system_instance/character_choose_system_instance.h"
#include "src/client/system_instance/character_create_system_instance.h"
#include "src/client/system_instance/game_save_system_instance.h"
#include "src/client/system_instance/login_notice_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "text_input_ui_system.h"
#include "wz/Property.h"
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

void CharacterCreateUiSystem::RenderButton() {
  const static std::array buttons_nodes = {
      WzResource::ui_->find(u"Login.img/NewChar/button:ok"),
      WzResource::ui_->find(u"Login.img/NewChar/button:cancel"),
  };
  auto pos = LoadPos();
  auto &camera = CameraGameInstance::camera_;
  std::array buttons_rect = {
      SDL_FRect{66 - camera.x, -1154 - camera.y, 104, 52},
      SDL_FRect{160 - camera.x, -1156 - camera.y, 104, 52},
  };
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos_;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) &&
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

SDL_FPoint CharacterCreateUiSystem::LoadPos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto &camera = CameraGameInstance::camera_;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void CharacterCreateUiSystem::RenderBackgrnd() {
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

void CharacterCreateUiSystem::RenderStat() {
  static auto t = WzResource::LoadTexture(
      WzResource::ui_->find(u"Login.img/NewChar/stat/canvas:statback"));
  auto &camera = CameraGameInstance::camera_;
  SDL_FRect pos_rect{
      70 - camera.x,
      -1322 - camera.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);

  const static std::array buttons_nodes = {
      WzResource::ui_->find(u"Login.img/NewChar/stat/button:str_prev"),
      WzResource::ui_->find(u"Login.img/NewChar/stat/button:str_next"),
      WzResource::ui_->find(u"Login.img/NewChar/stat/button:dex_prev"),
      WzResource::ui_->find(u"Login.img/NewChar/stat/button:dex_next"),
      WzResource::ui_->find(u"Login.img/NewChar/stat/button:int_prev"),
      WzResource::ui_->find(u"Login.img/NewChar/stat/button:int_next"),
      WzResource::ui_->find(u"Login.img/NewChar/stat/button:luk_prev"),
      WzResource::ui_->find(u"Login.img/NewChar/stat/button:luk_next"),
  };
  std::array buttons_rect = {
      SDL_FRect{pos_rect.x + 48, pos_rect.y + 4, 16, 17},
      SDL_FRect{pos_rect.x + 96, pos_rect.y + 4, 16, 17},
      SDL_FRect{pos_rect.x + 48, pos_rect.y + 27, 16, 17},
      SDL_FRect{pos_rect.x + 96, pos_rect.y + 27, 16, 17},
      SDL_FRect{pos_rect.x + 48, pos_rect.y + 50, 16, 17},
      SDL_FRect{pos_rect.x + 96, pos_rect.y + 50, 16, 17},
      SDL_FRect{pos_rect.x + 48, pos_rect.y + 73, 16, 17},
      SDL_FRect{pos_rect.x + 96, pos_rect.y + 73, 16, 17},
  };
  std::vector<bool> r = {
      str_point_ <= 4, str_point_ > 12 || remain_point_ == 0,
      dex_point_ <= 4, dex_point_ > 12 || remain_point_ == 0,
      int_point_ <= 4, int_point_ > 12 || remain_point_ == 0,
      luk_point_ <= 4, luk_point_ > 12 || remain_point_ == 0,
  };

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos_;
    if (r[i]) {
      auto d = WzResource::LoadTexture(k->find(u"disabled/0"));
      SDL_RenderTexture(Window::renderer_, d, nullptr, &pos_rect);
    } else if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) &&
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

  Freetype::LoadSize(13);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(0, 0, 0, 255);
  auto str1 = std::to_string(str_point_);
  auto str2 = std::u16string{str1.begin(), str1.end()};
  auto dx = Freetype::LoadW(str2);
  Freetype::DrawLine(str2, buttons_rect[0].x + 32 - dx / 2,
                      buttons_rect[0].y - 2);

  auto dex1 = std::to_string(dex_point_);
  auto dex2 = std::u16string{dex1.begin(), dex1.end()};
  dx = Freetype::LoadW(dex2);
  Freetype::DrawLine(dex2, buttons_rect[2].x + 32 - dx / 2,
                      buttons_rect[2].y - 2);

  auto int1 = std::to_string(int_point_);
  auto int2 = std::u16string{int1.begin(), int1.end()};
  dx = Freetype::LoadW(int2);
  Freetype::DrawLine(int2, buttons_rect[4].x + 32 - dx / 2,
                      buttons_rect[4].y - 2);

  auto luk1 = std::to_string(luk_point_);
  auto luk2 = std::u16string{luk1.begin(), luk1.end()};
  dx = Freetype::LoadW(luk2);
  Freetype::DrawLine(luk2, buttons_rect[6].x + 32 - dx / 2,
                      buttons_rect[6].y - 2);
  Freetype::LoadBold(true);
  Freetype::LoadSize(21);
  auto r1 = std::to_string(remain_point_);
  auto r2 = std::u16string{r1.begin(), r1.end()};
  Freetype::DrawLine(r2, buttons_rect[6].x + 98, buttons_rect[6].y - 28);
  Freetype::LoadBold(false);

  Freetype::LoadAligned(false);
}

void CharacterCreateUiSystem::RenderCharacter() {
  g_character_.pos.x = -84;
  g_character_.pos.y = -1211;
  // animate
  CharacterRenderSystem::RenderCharacter(g_character_);
}

void CharacterCreateUiSystem::RenderBanner() {
  static auto t = WzResource::LoadTexture(
      WzResource::ui_->find(u"Login.img/LoginStart/StepBanner/NewChar"));
  auto pos = LoadPos();
  SDL_FRect pos_rect{
      pos.x + 160,
      pos.y + 85,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);

  const static std::array buttons_nodes = {
      WzResource::ui_->find(u"Login.img/LoginStart/BtClassicPrev"),
  };
  std::array buttons_rect = {
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
  };
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos_;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
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

void CharacterCreateUiSystem::RenderCustomItem(float cx, float cy,
                                                    ChooseType type) {
  std::u16string item_name;
  SDL_FPoint pos;
  SDL_Texture *t;
  bool choose = false;
  switch (type) {
  case ChooseType::kGender: {
    if (!gender_) {
      auto str_node =
          WzResource::ms_->get_root()->find(u"String.img/Character/gender/0");
      item_name = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    } else {
      auto str_node =
          WzResource::ms_->get_root()->find(u"String.img/Character/gender/1");
      item_name = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    }
    if (choose_index_ == 0) {
      choose = true;
      t = WzResource::LoadTexture(WzResource::ui_->find(
          "Login.img/NewChar/customize/canvas:gender_selected"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui_->find("Login.img/NewChar/customize/canvas:gender"));
    }
    pos = {35, 30};
    break;
  }
  case ChooseType::kFace: {
    item_name = EquipGameInstance::LoadEquipName(g_character_.face.id);
    if (choose_index_ == 1) {
      choose = true;
      t = WzResource::LoadTexture(WzResource::ui_->find(
          "Login.img/NewChar/customize/canvas:face_selected"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui_->find("Login.img/NewChar/customize/canvas:face"));
    }
    pos = {35, 48};
    break;
  }
  case ChooseType::kHairstyle: {
    item_name = EquipGameInstance::LoadEquipName(g_character_.hair);
    if (choose_index_ == 2) {
      choose = true;
      t = WzResource::LoadTexture(WzResource::ui_->find(
          "Login.img/NewChar/customize/canvas:hairstyle_selected"));
    } else {
      t = WzResource::LoadTexture(WzResource::ui_->find(
          "Login.img/NewChar/customize/canvas:hairstyle"));
    }
    pos = {35, 66};
    break;
  }
  case ChooseType::kHaircolor: {
    static auto haircolor_node =
        WzResource::ui_->find(u"Login.img/HairColorName");
    auto back = g_character_.hair.back();
    auto node = haircolor_node->get_child(std::u16string{back});
    item_name = static_cast<wz::Property<std::u16string> *>(node)->get();
    if (choose_index_ == 3) {
      choose = true;
      t = WzResource::LoadTexture(WzResource::ui_->find(
          "Login.img/NewChar/customize/canvas:haircolor_selected"));
    } else {
      t = WzResource::LoadTexture(WzResource::ui_->find(
          "Login.img/NewChar/customize/canvas:haircolor"));
    }
    pos = {35, 84};
    break;
  }
  case ChooseType::kSkin: {
    item_name = EquipGameInstance::LoadEquipName(g_character_.head);
    if (choose_index_ == 4) {
      choose = true;
      t = WzResource::LoadTexture(WzResource::ui_->find(
          "Login.img/NewChar/customize/canvas:skincolor_selected"));
    } else {
      t = WzResource::LoadTexture(WzResource::ui_->find(
          "Login.img/NewChar/customize/canvas:skincolor"));
    }
    pos = {35, 102};
    break;
  }
  case ChooseType::kTop: {
    item_name = EquipGameInstance::LoadEquipName(g_character_.coat->id);
    if (choose_index_ == 5) {
      choose = true;
      t = WzResource::LoadTexture(WzResource::ui_->find(
          "Login.img/NewChar/customize/canvas:top_selected"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui_->find("Login.img/NewChar/customize/canvas:top"));
    }
    pos = {35, 120};
    break;
  }
  case ChooseType::kBottom: {
    item_name = EquipGameInstance::LoadEquipName(g_character_.pant->id);
    if (choose_index_ == 6) {
      choose = true;
      t = WzResource::LoadTexture(WzResource::ui_->find(
          "Login.img/NewChar/customize/canvas:bottom_selected"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui_->find("Login.img/NewChar/customize/canvas:bottom"));
    }
    pos = {35, 138};
    break;
  }
  case ChooseType::kShoes: {
    item_name = EquipGameInstance::LoadEquipName(g_character_.shoes->id);
    if (choose_index_ == 7) {
      choose = true;
      t = WzResource::LoadTexture(WzResource::ui_->find(
          "Login.img/NewChar/customize/canvas:shoes_selected"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui_->find("Login.img/NewChar/customize/canvas:shoes"));
    }
    pos = {35, 156};
    break;
  }
  case ChooseType::kWeapon: {
    item_name = EquipGameInstance::LoadEquipName(g_character_.weapon->id);
    if (choose_index_ == 8) {
      choose = true;
      t = WzResource::LoadTexture(WzResource::ui_->find(
          "Login.img/NewChar/customize/canvas:weapon_selected"));
    } else {
      t = WzResource::LoadTexture(
          WzResource::ui_->find("Login.img/NewChar/customize/canvas:weapon"));
    }
    pos = {35, 174};
    break;
  }
  }
  SDL_FRect pos_rect{
      cx + pos.x,
      cy + pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);

  Freetype::LoadSize(13);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(0, 0, 0, 255);
  auto dx = Freetype::LoadW(item_name);
  Freetype::DrawLine(item_name, pos_rect.x + 175 - dx / 2, pos_rect.y);
  Freetype::LoadAligned(false);

  static auto prev_button =
      WzResource::ui_->find("Login.img/NewChar/customize/button:gender_prev");

  static auto next_button =
      WzResource::ui_->find("Login.img/NewChar/customize/button:gender_next");
  if (choose) {
    std::vector<wz::Node *> buttons_nodes = {prev_button, next_button};
    std::vector<SDL_FRect> buttons_rect = {
        SDL_FRect{cx + pos.x + 95, cy + pos.y, 16, 17},  //
        SDL_FRect{cx + pos.x + 235, cy + pos.y, 16, 17}, //
    };

    for (size_t i = 0; i < buttons_nodes.size(); ++i) {
      auto k = buttons_nodes[i];
      auto pos_rect = buttons_rect[i];
      pos_rect.x = (int)pos_rect.x;
      pos_rect.y = (int)pos_rect.y;
      auto &mouse_pos = Window::mouse_pos_;
      if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) &&
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
}

void CharacterCreateUiSystem::RenderCustom() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"Login.img/NewChar/customize/animOpen/3"));
  auto pos = LoadPos();
  auto &camera = CameraGameInstance::camera_;
  auto cx = -440 - camera.x;
  auto cy = -1370 - camera.y;
  SDL_FRect pos_rect{
      cx,
      cy,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);

  RenderCustomItem(cx, cy, ChooseType::kGender);
  RenderCustomItem(cx, cy, ChooseType::kFace);
  RenderCustomItem(cx, cy, ChooseType::kHairstyle);
  RenderCustomItem(cx, cy, ChooseType::kHaircolor);
  RenderCustomItem(cx, cy, ChooseType::kSkin);
  RenderCustomItem(cx, cy, ChooseType::kTop);
  RenderCustomItem(cx, cy, ChooseType::kBottom);
  RenderCustomItem(cx, cy, ChooseType::kShoes);
  RenderCustomItem(cx, cy, ChooseType::kWeapon);
}

void CharacterCreateUiSystem::RenderName() {
  TextInputUiSystem::Render(name_, 5, 5);
}

bool CharacterCreateUiSystem::Render() {
  RenderStat();
  RenderButton();
  RenderCharacter();
  RenderCustom();
  RenderBackgrnd();
  RenderBanner();
  RenderName();
  return true;
}

bool CharacterCreateUiSystem::BackAnimate() {
  TextInputUiSystem::Close(name_);
  if (LoginUiSystem::CameraAnimate(-80, -479)) {
    CharacterChooseSystemInstance::Enter();
    return false;
  }
  return true;
}

void CharacterCreateUiSystem::EventButtonBack() {
  AudioGameInstance::LoadAudio(u"UI.img/ScrollUp", 0);
  CharacterChooseUiSystem::choose_ = std::nullopt;

  CharacterCreateSystemInstance::Enter();
  System::logic_systems_.push_back(BackAnimate);
  System::render_systems_ = {
      LoginSystemInstance::RenderGame,
      CharacterChooseUiSystem::Render,
      CursorRenderSystem::Render,
  };
  System::event_systems_ = {};
}

std::vector<std::u16string> CharacterCreateUiSystem::LoadHaircolors() {
  std::vector<std::u16string> r;
  static auto haircolors = WzResource::ui_->find(u"Login.img/HairColorName");
  for (auto [k, v] : *haircolors->get_children()) {
    r.push_back(static_cast<wz::Property<std::u16string> *>(v[0])->get());
  }
  return r;
}

void CharacterCreateUiSystem::ResetCharacter(bool g) {
  gender_ = g;

  auto &g_character = CharacterCreateUiSystem::g_character_;
  CharacterGameInstance::AddBody(g_character, u"00002000");
  CharacterGameInstance::AddHead(g_character, u"00012000");
  CharacterGameInstance::AddEar(g_character, u"humanEar");
  CharacterGameInstance::AddFace(g_character, LoadDefaultFace()[0]);
  CharacterGameInstance::AddHair(g_character, LoadDefaultHair()[0]);
  CharacterGameInstance::AddCoat(g_character, LoadDefaultTop()[0]);
  CharacterGameInstance::AddPants(g_character, LoadDefaultBottom()[0]);
  CharacterGameInstance::AddShoes(g_character, LoadDefaultShoes()[0]);

  CharacterGameInstance::AddWeapon(g_character, LoadDefaultWeapon()[0]);
  CharacterLogicSystem::RunStandAction(g_character);
  g_character.flip = 1;
  g_character.name = u"";
}

std::vector<std::u16string> CharacterCreateUiSystem::LoadDefaultFace() {
  std::vector<std::u16string> faces;
  if (!gender_) {
    faces = {u"00020000", u"00020001", u"00020002"};
  } else {
    faces = {u"00021000", u"00021001", u"00021002"};
  }
  return faces;
}

void CharacterCreateUiSystem::EventButtonFacePrev() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto faces = LoadDefaultFace();
  auto face_id = g_character_.face.id;
  auto it = std::ranges::find(faces, face_id);
  if (it != faces.end()) {
    // 计算前一个迭代器（循环）
    auto prev_it = (it == faces.begin()) ? (faces.end() - 1) : (it - 1);
    CharacterGameInstance::AddFace(g_character_, *prev_it);
  }
}

void CharacterCreateUiSystem::EventButtonFaceNext() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto faces = LoadDefaultFace();
  auto face_id = g_character_.face.id;
  auto it = std::ranges::find(faces, face_id);
  if (it != faces.end()) {
    auto next_it = (it + 1 == faces.end()) ? faces.begin() : (it + 1);
    CharacterGameInstance::AddFace(g_character_, *next_it);
  }
}

std::vector<std::u16string> CharacterCreateUiSystem::LoadDefaultHair() {
  std::vector<std::u16string> hairs;
  if (!gender_) {
    hairs = {u"00030000", u"00030020", u"00030030"};
  } else {
    hairs = {u"00031000", u"00031040", u"00031050"};
  }
  return hairs;
}

void CharacterCreateUiSystem::EventButtonHairPrev() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto hairs = LoadDefaultHair();
  auto hair_id = g_character_.hair;
  hair_id.back() = u'0';
  auto it = std::ranges::find(hairs, hair_id);
  if (it != hairs.end()) {
    auto prev_it = (it == hairs.begin()) ? (hairs.end() - 1) : (it - 1);
    CharacterGameInstance::AddHair(g_character_, *prev_it);
  }
}

void CharacterCreateUiSystem::EventButtonHairNext() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto hairs = LoadDefaultHair();
  auto hair_id = g_character_.hair;
  hair_id.back() = u'0';
  auto it = std::ranges::find(hairs, hair_id);
  if (it != hairs.end()) {
    auto next_it = (it + 1 == hairs.end()) ? hairs.begin() : (it + 1);
    CharacterGameInstance::AddHair(g_character_, *next_it);
  }
}

void CharacterCreateUiSystem::EventButtonHairColorPrev() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto hair_id = g_character_.hair;
  auto back = hair_id.back();
  if (back > u'0') {
    back -= 1;
  } else {
    back = u'7';
  }
  hair_id.back() = back;
  CharacterGameInstance::AddHair(g_character_, hair_id);
}

void CharacterCreateUiSystem::EventButtonHairColorNext() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto hair_id = g_character_.hair;
  auto back = hair_id.back();
  if (back < u'7') {
    back += 1;
  } else {
    back = u'0';
  }
  hair_id.back() = back;
  CharacterGameInstance::AddHair(g_character_, hair_id);
}

void CharacterCreateUiSystem::EventButtonSkinColorPrev() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto head_id = g_character_.head;
  auto body_id = g_character_.body;
  auto back = head_id.back();
  if (back > u'0') {
    back -= 1;
  } else {
    back = u'3';
  }
  head_id.back() = back;
  body_id.back() = back;
  CharacterGameInstance::AddHead(g_character_, head_id);
  CharacterGameInstance::AddBody(g_character_, body_id);
}

void CharacterCreateUiSystem::EventButtonSkinColorNext() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto &head_id = g_character_.head;
  auto body_id = g_character_.body;
  auto back = head_id.back();
  if (back < u'3') {
    back += 1;
  } else {
    back = u'0';
  }
  head_id.back() = back;
  body_id.back() = back;
  CharacterGameInstance::AddHead(g_character_, head_id);
  CharacterGameInstance::AddBody(g_character_, body_id);
}

std::vector<std::u16string> CharacterCreateUiSystem::LoadDefaultTop() {
  std::vector<std::u16string> tops;
  if (!gender_) {
    tops = {u"01040001", u"01040002", u"01040003"};
  } else {
    tops = {u"01041001", u"01041002", u"01041003", u"01041004"};
  }
  return tops;
}

void CharacterCreateUiSystem::EventButtonTopPrev() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto tops = LoadDefaultTop();
  auto top_id = g_character_.coat->id;
  auto it = std::ranges::find(tops, top_id);
  if (it != tops.end()) {
    auto prev_it = (it == tops.begin()) ? (tops.end() - 1) : (it - 1);
    CharacterGameInstance::AddCoat(g_character_, *prev_it);
  }
}

void CharacterCreateUiSystem::EventButtonTopNext() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto tops = LoadDefaultTop();
  auto top_id = g_character_.coat->id;
  auto it = std::ranges::find(tops, top_id);
  if (it != tops.end()) {
    auto next_it = (it + 1 == tops.end()) ? tops.begin() : (it + 1);
    CharacterGameInstance::AddCoat(g_character_, *next_it);
  }
}

std::vector<std::u16string> CharacterCreateUiSystem::LoadDefaultBottom() {
  std::vector<std::u16string> bottom;
  if (!gender_) {
    bottom = {u"01060001", u"01060002"};
  } else {
    bottom = {u"01061001", u"01061002"};
  }
  return bottom;
}

void CharacterCreateUiSystem::EventButtonBottomPrev() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto bottoms = LoadDefaultBottom();
  auto pant_id = g_character_.pant->id;
  auto it = std::ranges::find(bottoms, pant_id);
  if (it != bottoms.end()) {
    auto prev_it = (it == bottoms.begin()) ? (bottoms.end() - 1) : (it - 1);
    CharacterGameInstance::AddPants(g_character_, *prev_it);
  }
}

void CharacterCreateUiSystem::EventButtonBottomNext() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto bottoms = LoadDefaultBottom();
  auto pant_id = g_character_.pant->id;
  auto it = std::ranges::find(bottoms, pant_id);
  if (it != bottoms.end()) {
    auto next_it = (it + 1 == bottoms.end()) ? bottoms.begin() : (it + 1);
    CharacterGameInstance::AddPants(g_character_, *next_it);
  }
}

std::vector<std::u16string> CharacterCreateUiSystem::LoadDefaultShoes() {
  std::vector<std::u16string> shoes;
  shoes = {u"01072000", u"01072001", u"01072002", u"01072003"};
  return shoes;
}

void CharacterCreateUiSystem::EventButtonShoesPrev() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto shoes = LoadDefaultShoes();
  auto shoes_id = g_character_.shoes->id;
  auto it = std::ranges::find(shoes, shoes_id);
  if (it != shoes.end()) {
    auto prev_it = (it == shoes.begin()) ? (shoes.end() - 1) : (it - 1);
    CharacterGameInstance::AddShoes(g_character_, *prev_it);
  }
}

void CharacterCreateUiSystem::EventButtonShoesNext() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto shoes = LoadDefaultShoes();
  auto shoes_id = g_character_.shoes->id;
  auto it = std::ranges::find(shoes, shoes_id);
  if (it != shoes.end()) {
    auto next_it = (it + 1 == shoes.end()) ? shoes.begin() : (it + 1);
    CharacterGameInstance::AddShoes(g_character_, *next_it);
  }
}

std::vector<std::u16string> CharacterCreateUiSystem::LoadDefaultWeapon() {
  std::vector<std::u16string> weapon;
  weapon = {u"01302000", u"01312000", u"01322000"};
  return weapon;
}

void CharacterCreateUiSystem::EventButtonWeaponPrev() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto weapons = LoadDefaultWeapon();
  auto weapon_id = g_character_.weapon->id;
  auto it = std::ranges::find(weapons, weapon_id);
  if (it != weapons.end()) {
    auto prev_it = (it == weapons.begin()) ? (weapons.end() - 1) : (it - 1);
    CharacterGameInstance::AddWeapon(g_character_, *prev_it);
  }
}

void CharacterCreateUiSystem::EventButtonWeaponNext() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);

  auto weapons = LoadDefaultWeapon();
  auto weapon_id = g_character_.weapon->id;
  auto it = std::ranges::find(weapons, weapon_id);
  if (it != weapons.end()) {
    auto next_it = (it + 1 == weapons.end()) ? weapons.begin() : (it + 1);
    CharacterGameInstance::AddWeapon(g_character_, *next_it);
  }
}

void CharacterCreateUiSystem::EventButtonStrInc() {
  if (remain_point_ > 0) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
    str_point_++;
    remain_point_--;
  }
}

void CharacterCreateUiSystem::EventButtonStrDec() {
  if (str_point_ > 4) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
    str_point_--;
    remain_point_++;
  }
}

void CharacterCreateUiSystem::EventButtonDexInc() {
  if (remain_point_ > 0) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
    dex_point_++;
    remain_point_--;
  }
}

void CharacterCreateUiSystem::EventButtonDexDec() {
  if (dex_point_ > 4) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
    dex_point_--;
    remain_point_++;
  }
}

void CharacterCreateUiSystem::EventButtonIntInc() {
  if (remain_point_ > 0) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
    int_point_++;
    remain_point_--;
  }
}

void CharacterCreateUiSystem::EventButtonIntDec() {
  if (int_point_ > 4) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
    int_point_--;
    remain_point_++;
  }
}

void CharacterCreateUiSystem::EventButtonLukInc() {
  if (remain_point_ > 0) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
    luk_point_++;
    remain_point_--;
  }
}

void CharacterCreateUiSystem::EventButtonLukDec() {
  if (luk_point_ > 4) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
    luk_point_--;
    remain_point_++;
  }
}

bool CharacterCreateUiSystem::EventButtonCustom(SDL_Event *Event) {
  auto &camera = CameraGameInstance::camera_;
  auto cx = -440 - camera.x;
  auto cy = -1370 - camera.y;
  cx += 35;
  cy += 30;
  for (uint8_t i = 0; i <= 8; i++) {
    SDL_FRect pos_rect{cx, cy + 18 * i, 256, 17};
    auto &mouse_pos = Window::mouse_pos_;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (choose_index_ != i) {
        choose_index_ = i;
        return true;
      } else {
        std::vector<SDL_FRect> buttons_rect = {
            SDL_FRect{cx + 95, cy + 18 * i, 16, 17},  //
            SDL_FRect{cx + 235, cy + 18 * i, 16, 17}, //
        };
        std::optional<bool> left;
        if (SDL_PointInRectFloat(&mouse_pos, &buttons_rect[0])) {
          left = true;
        } else if (SDL_PointInRectFloat(&mouse_pos, &buttons_rect[1])) {
          left = false;
        }
        if (left.has_value()) {
          switch (choose_index_) {
          case 0: {
            AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
            ResetCharacter(!gender_);
            break;
          }
          case 1: {
            // face
            if (left.value()) {
              EventButtonFacePrev();
            } else {
              EventButtonFaceNext();
            }
            break;
          }
          case 2: {
            if (left.value()) {
              EventButtonHairPrev();
            } else {
              EventButtonHairNext();
            }
            break;
          }
          case 3: {
            if (left.value()) {
              EventButtonHairColorPrev();
            } else {
              EventButtonHairColorNext();
            }
            break;
          }
          case 4: {
            if (left.value()) {
              EventButtonSkinColorPrev();
            } else {
              EventButtonSkinColorNext();
            }
            break;
          }
          case 5: {
            if (left.value()) {
              EventButtonTopPrev();
            } else {
              EventButtonTopNext();
            }
            break;
          }
          case 6: {
            if (left.value()) {
              EventButtonBottomPrev();
            } else {
              EventButtonBottomNext();
            }
            break;
          }
          case 7: {
            if (left.value()) {
              EventButtonShoesPrev();
            } else {
              EventButtonShoesNext();
            }
            break;
          }
          case 8: {
            if (left.value()) {
              EventButtonWeaponPrev();
            } else {
              EventButtonWeaponNext();
            }
            break;
          }
          }
        }
      }
    }
  }
  return false;
}

void CharacterCreateUiSystem::EventButtonOk() {
  // check size
  auto &characters = CharacterChooseUiSystem::characters_;
  if (characters.size() >= 3) {
    LoginNoticeSystemInstance::Enter(
        LoginNoticeSystemInstance::NoticeType::kCharacterFull, nullptr);
    AudioGameInstance::LoadAudio(u"UI.img/DlgNotice", 0);
    return;
  }
  // check name
  auto &name_text = name_.text;
  if (name_text.empty()) {
    LoginNoticeSystemInstance::Enter(
        LoginNoticeSystemInstance::NoticeType::kCharacternameError, nullptr);
    AudioGameInstance::LoadAudio(u"UI.img/DlgNotice", 0);
    return;
  }
  // check ap
  if (remain_point_ != 0) {
    LoginNoticeSystemInstance::Enter(
        LoginNoticeSystemInstance::NoticeType::kCharacterUseAp, nullptr);
    AudioGameInstance::LoadAudio(u"UI.img/DlgNotice", 0);
    return;
  }
  for (auto ch : name_text) {
    if (SDL_ispunct(static_cast<int>(ch)) ||
        SDL_isspace(static_cast<int>(ch))) {
      // 发现标点符号
      LoginNoticeSystemInstance::Enter(
          LoginNoticeSystemInstance::NoticeType::kCharacternameError, nullptr);
      AudioGameInstance::LoadAudio(u"UI.img/DlgNotice", 0);
      return;
    }
  }
  for (auto g : characters) {
    if (g.name == name_text) {
      // 名称重复
      LoginNoticeSystemInstance::Enter(
          LoginNoticeSystemInstance::NoticeType::kCharacternameUsed, nullptr);
      AudioGameInstance::LoadAudio(u"UI.img/DlgNotice", 0);
      return;
    }
  }
  AudioGameInstance::LoadAudio(u"UI.img/ScrollUp", 0);

  g_character_.name = name_text;
  characters.push_back(g_character_);

  CharacterSaveData cs;
  cs.character = g_character_;
  cs.ap.hp_ap = 0;
  cs.ap.mp_ap = 0;

  cs.ap.str_ap = str_point_;
  cs.ap.dex_ap = dex_point_;
  cs.ap.int_ap = int_point_;
  cs.ap.luk_ap = luk_point_;

  cs.map_id = 1;

  GameSaveSystemInstance::save_.characters.push_back(std::move(cs));

  CharacterCreateSystemInstance::Enter();
  EventButtonBack();
}

void CharacterCreateUiSystem::EventButtonCancel() { EventButtonBack(); }

bool CharacterCreateUiSystem::EventButton(SDL_Event *Event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto &camera = CameraGameInstance::camera_;
  auto pos = LoadPos();
  SDL_FPoint stat_point{
      70 - camera.x,
      -1322 - camera.y,
  };
  r = {
      SDL_FRect{stat_point.x + 48, stat_point.y + 4, 16, 17},
      SDL_FRect{stat_point.x + 96, stat_point.y + 4, 16, 17},
      SDL_FRect{stat_point.x + 48, stat_point.y + 27, 16, 17},
      SDL_FRect{stat_point.x + 96, stat_point.y + 27, 16, 17},
      SDL_FRect{stat_point.x + 48, stat_point.y + 50, 16, 17},
      SDL_FRect{stat_point.x + 96, stat_point.y + 50, 16, 17},
      SDL_FRect{stat_point.x + 48, stat_point.y + 73, 16, 17},
      SDL_FRect{stat_point.x + 96, stat_point.y + 73, 16, 17},
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
      SDL_FRect{66 - camera.x, -1154 - camera.y, 104, 52},
      SDL_FRect{160 - camera.x, -1156 - camera.y, 104, 52},

  };
  fns = {
      EventButtonStrDec, EventButtonStrInc, EventButtonDexDec,
      EventButtonDexInc, EventButtonIntDec, EventButtonIntInc,
      EventButtonLukDec, EventButtonLukInc, EventButtonBack,
      EventButtonOk,      EventButtonCancel,
  };
  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      fns[i]();
      return false;
    }
  }

  return false;
}

bool CharacterCreateUiSystem::Event(SDL_Event *Event) {
  TextInputUiSystem::Event(Event, name_);
  bool r = true;
  switch (Event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = Event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      return false;
      break;
    }
    default: {
      break;
    }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      r = false;
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      EventButton(Event);
      EventButtonCustom(Event);
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

bool CharacterCreateUiSystem::Run() {
  CharacterLogicSystem::RunAnimate(g_character_);
  return true;
}