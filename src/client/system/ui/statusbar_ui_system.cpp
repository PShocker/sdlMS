#include "src/client/system/ui/statusbar_ui_system.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include "src/client/system/ui/character_stat_ui_system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/scroll_ui_system.h"
#include "src/client/system/ui/skill_ui_system.h"
#include "src/client/game/game_chat.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/chat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/chat_log_ui_system.h"
#include "src/client/system/ui/keybinding_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/client/system/ui/statusbar_menu_ui_system.h"
#include "src/client/system/ui/text_input_ui_system.h"
#include <wz/Node.h>
#include <wz/Property.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <flat_map>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <vector>
namespace sdlms {


void StatusbarUiSystem::RenderHmAlert() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/backgrnd2"));

  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - backgrnd->w) / 2;
  auto base_y = (screen_h - backgrnd->h);

  auto hp = CharacterStatGameInstance::GetHp();
  auto max_hp = CharacterStatGameInstance::GetHpMax();
  auto hp_percent_now = (float)hp / max_hp;
  static auto hp_percent = hp_percent_now;
  hp_percent = std::lerp(hp_percent, hp_percent_now, 0.05);
  auto diff = std::abs(hp_percent - hp_percent_now);
  if (diff < std::pow(10, -4)) {
    hp_percent = hp_percent_now;
  }
  if (hp_percent_now < hp_percent && hp_percent <= 0.3) {
    auto now = Window::dt_now_;
    auto sum = 5 * 130;
    auto offset = now % sum; // 取余，得到周期内偏移
    auto i = offset / 130;
    auto node = WzResource::ui_->find(u"StatusBar.img/alertHP");
    node = node->get_child(std::to_string(i));
    auto icon = WzResource::LoadTexture(node);
    SDL_FRect pos_rect{
        base_x + 223,
        base_y + 54,
        static_cast<float>(icon->w),
        static_cast<float>(icon->h),
    };
    SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
  }

  auto mp = CharacterStatGameInstance::GetMp();
  auto max_mp = CharacterStatGameInstance::GetMpMax();
  auto mp_percent_now = (float)mp / max_mp;
  static auto mp_percent = mp_percent_now;
  mp_percent = std::lerp(mp_percent, mp_percent_now, 0.05);
  diff = std::abs(mp_percent - mp_percent_now);
  if (diff < std::pow(10, -4)) {
    mp_percent = mp_percent_now;
  }
  if (mp_percent_now < mp_percent && mp_percent <= 0.3) {
    auto now = Window::dt_now_;
    auto sum = 5 * 130;
    auto offset = now % sum; // 取余，得到周期内偏移
    auto i = offset / 130;
    auto node = WzResource::ui_->find(u"StatusBar.img/alertMP");
    node = node->get_child(std::to_string(i));
    auto icon = WzResource::LoadTexture(node);
    SDL_FRect pos_rect{
        base_x + 331,
        base_y + 54,
        static_cast<float>(icon->w),
        static_cast<float>(icon->h),
    };
    SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
  }
}

void StatusbarUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/backgrnd2"));
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - backgrnd->w) / 2;
  auto base_y = (screen_h - backgrnd->h);
  SDL_FRect pos_rect = {base_x, base_y, static_cast<float>(backgrnd->w),
                        static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);

  static auto backgrnd2 = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/canvas:backgrnd1"));
  pos_rect = {base_x + 7, base_y + 9, static_cast<float>(backgrnd2->w),
              static_cast<float>(backgrnd2->h)};
  SDL_RenderTexture(Window::renderer_, backgrnd2, nullptr, &pos_rect);
  static auto gaugeLabel = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/canvas:gaugeLabel"));
  pos_rect = {base_x + 223, base_y + 40, static_cast<float>(gaugeLabel->w),
              static_cast<float>(gaugeLabel->h)};
  SDL_RenderTexture(Window::renderer_, gaugeLabel, nullptr, &pos_rect);
}

void StatusbarUiSystem::RenderGaugeText() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/backgrnd2"));

  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - backgrnd->w) / 2;
  auto base_y = (screen_h - backgrnd->h);
  static auto num_node = WzResource::ui_->find(u"StatusBar.img/gauge/number");
  std::array<int32_t, 2> a = {
      CharacterStatGameInstance::GetHp(),
      CharacterStatGameInstance::GetMp(),
  };
  std::array<int32_t, 2> a2 = {
      CharacterStatGameInstance::GetHpMax(),
      CharacterStatGameInstance::GetMpMax(),
  };
  std::array<SDL_FPoint, 2> a3 = {
      SDL_FPoint{245, 42},
      SDL_FPoint{356, 42},
  };
  SDL_FRect pos_rect;
  for (auto i : {0, 1}) {
    auto l_bracket = WzResource::LoadTexture(num_node->get_child(u"["));
    auto r_bracket = WzResource::LoadTexture(num_node->get_child(u"]"));
    pos_rect = {base_x + a3[i].x, base_y + a3[i].y,
                static_cast<float>(l_bracket->w),
                static_cast<float>(l_bracket->h)};
    SDL_RenderTexture(Window::renderer_, l_bracket, nullptr, &pos_rect);
    int32_t w = 6;
    a[i] = std::max(a[i], 0);
    auto num = std::to_string(a[i]);
    for (auto c : num) {
      pos_rect.x = base_x + w + a3[i].x;
      auto texture =
          WzResource::LoadTexture(num_node->get_child(std::string{c}));
      pos_rect.w = texture->w;
      pos_rect.h = texture->h;
      SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
      w += 6;
    }
    auto texture = WzResource::LoadTexture(num_node->get_child(u"slash"));
    pos_rect.x = base_x + w + a3[i].x;
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
    w += 6;

    num = std::to_string(a2[i]);
    for (auto c : num) {
      pos_rect.x = base_x + w + a3[i].x;
      auto texture =
          WzResource::LoadTexture(num_node->get_child(std::string{c}));
      pos_rect.w = texture->w;
      pos_rect.h = texture->h;
      SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
      w += 6;
    }

    pos_rect.x = base_x + w + a3[i].x;
    pos_rect.w = r_bracket->w;
    pos_rect.h = r_bracket->h;
    SDL_RenderTexture(Window::renderer_, r_bracket, nullptr, &pos_rect);
  }
  SDL_FPoint a4{473, 42};
  auto self_exp = CharacterStatGameInstance::GetExp();
  auto self_max_exp = CharacterStatGameInstance::GetExpMax();
  float self_exp_percent = (float)self_exp / self_max_exp;
  auto num = std::to_string(self_exp);
  int32_t w = 0;
  for (auto c : num) {
    pos_rect.x = base_x + w + a4.x;
    auto texture =
        WzResource::LoadTexture(num_node->get_child(std::string{c}));
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
    w += 6;
  }
  auto l_bracket = WzResource::LoadTexture(num_node->get_child(u"["));
  auto r_bracket = WzResource::LoadTexture(num_node->get_child(u"]"));

  pos_rect.x += 8;
  SDL_RenderTexture(Window::renderer_, l_bracket, nullptr, &pos_rect);

  auto exp_percent_num = std::format("{:.2f}", self_exp_percent * 100);
  for (auto c : exp_percent_num) {
    auto texture =
        WzResource::LoadTexture(num_node->get_child(std::string{c}));
    pos_rect.x += 6;
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
  auto percent_texture = WzResource::LoadTexture(num_node->get_child(u"%"));
  pos_rect.x += 6;
  pos_rect.w = percent_texture->w;
  pos_rect.h = percent_texture->h;
  SDL_RenderTexture(Window::renderer_, percent_texture, nullptr, &pos_rect);

  pos_rect.x += 8;
  pos_rect.w = r_bracket->w;
  pos_rect.h = r_bracket->h;
  SDL_RenderTexture(Window::renderer_, r_bracket, nullptr, &pos_rect);
}

void StatusbarUiSystem::RenderCharacterStat() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/backgrnd2"));

  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - backgrnd->w) / 2;
  auto base_y = (screen_h - backgrnd->h);

  static auto gaugeHp = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/gauge/layer:gaugeHp"));

  static auto gray = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/gauge/canvas:gaugeBack"));

  auto hp = CharacterStatGameInstance::GetHp();
  auto max_hp = CharacterStatGameInstance::GetHpMax();
  auto hp_percent_now = (float)hp / max_hp;
  static auto hp_percent = hp_percent_now;
  SDL_FRect pos_rect;
  pos_rect = {
      base_x + 224,
      base_y + 56,
      static_cast<float>(gaugeHp->w),
      static_cast<float>(gray->h),
  };
  SDL_RenderTexture(Window::renderer_, gray, nullptr, &pos_rect);

  hp_percent = std::lerp(hp_percent, hp_percent_now, 0.05);
  SDL_FRect src_rect{
      0,
      0,
      static_cast<float>(gaugeHp->w * hp_percent),
      static_cast<float>(gaugeHp->h),
  };
  pos_rect = {
      base_x + 224,
      base_y + 56,
      static_cast<float>(src_rect.w),
      static_cast<float>(gaugeHp->h),
  };
  SDL_RenderTexture(Window::renderer_, gaugeHp, &src_rect, &pos_rect);

  auto mp = CharacterStatGameInstance::GetMp();
  auto max_mp = CharacterStatGameInstance::GetMpMax();
  auto mp_percent = (float)mp / max_mp;
  static auto gaugeMp = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/gauge/layer:gaugeMp"));
  pos_rect = {
      base_x + 332,
      base_y + 56,
      static_cast<float>(gaugeMp->w),
      static_cast<float>(gray->h),
  };
  SDL_RenderTexture(Window::renderer_, gray, nullptr, &pos_rect);

  src_rect = {
      0,
      0,
      static_cast<float>(gaugeMp->w * mp_percent),
      static_cast<float>(gaugeMp->h),
  };

  pos_rect = {base_x + 332, base_y + 56, static_cast<float>(src_rect.w),
              static_cast<float>(gaugeMp->h)};
  SDL_RenderTexture(Window::renderer_, gaugeMp, &src_rect, &pos_rect);

  static auto gaugeExp = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/gauge/layer:gaugeExp"));
  auto exp = CharacterStatGameInstance::GetExp();
  auto max_exp = CharacterStatGameInstance::GetExpMax();
  auto exp_percent = (float)exp / max_exp;
  exp_percent = std::min(1.0f, exp_percent);

  pos_rect = {
      base_x + 445,
      base_y + 56,
      static_cast<float>(gaugeExp->w),
      static_cast<float>(gray->h),
  };
  SDL_RenderTexture(Window::renderer_, gray, nullptr, &pos_rect);

  src_rect = {
      0,
      0,
      static_cast<float>(gaugeExp->w * exp_percent),
      static_cast<float>(gaugeExp->h),
  };

  pos_rect = {base_x + 445, base_y + 56, static_cast<float>(src_rect.w),
              static_cast<float>(gaugeExp->h)};
  SDL_RenderTexture(Window::renderer_, gaugeExp, nullptr, &pos_rect);

  static auto gaugeCover = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/layer:gaugeCover"));
  pos_rect = {base_x + 223, base_y + 55, static_cast<float>(gaugeCover->w),
              static_cast<float>(gaugeCover->h)};
  SDL_RenderTexture(Window::renderer_, gaugeCover, nullptr, &pos_rect);

  // job
  auto job_node = WzResource::ms_->get_root()->find(
      u"String.img/Job/" + CharacterGameInstance::self_.job);
  auto job_name = static_cast<wz::Property<std::u16string> *>(job_node)->get();
  Freetype::LoadSize(12);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::LoadAligned(true);
  Freetype::DrawLine(job_name, base_x + 88, base_y + 36);
  // name
  const auto &self = CharacterGameInstance::self_;
  auto &self_name = self.name;
  Freetype::LoadSize(12);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(self_name, base_x + 88, base_y + 52);
  Freetype::LoadAligned(false);

  // level
  static auto lvNumber = WzResource::ui_->find(u"StatusBar.img/lvNumber");
  auto level = self.level;
  auto level_str = std::to_string(level);
  pos_rect.x = base_x + 44;
  pos_rect.y = base_y + 49;
  int32_t w = 0;
  int digits = level == 0 ? 1 : static_cast<int>(std::log10(level)) + 1;
  pos_rect.x += (36 - digits * 12) / (digits + 1);
  for (auto c : level_str) {
    auto t = WzResource::LoadTexture(lvNumber->get_child(std::string{c}));
    pos_rect.x += w;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
    w = 12;
  }

  RenderGaugeText();
}

void StatusbarUiSystem::RenderButton() {
  static auto CashShop =
      WzResource::ui_->find(u"StatusBar.img/button:CashShop");
  static auto Menu = WzResource::ui_->find(u"StatusBar.img/button:Menu");
  static auto Shortcut =
      WzResource::ui_->find(u"StatusBar.img/button:Shortcut");
  static auto Mailbox = WzResource::ui_->find(u"StatusBar.img/button:Mailbox");
  static auto equip = WzResource::ui_->find(u"StatusBar.img/button:Equip");
  static auto Inven = WzResource::ui_->find(u"StatusBar.img/button:Inven");
  static auto Stat = WzResource::ui_->find(u"StatusBar.img/button:Stat");
  static auto StatUp = WzResource::ui_->find(u"StatusBar.img/button:StatUp");
  static auto Skill = WzResource::ui_->find(u"StatusBar.img/button:Skill");
  static auto SkillUp = WzResource::ui_->find(u"StatusBar.img/button:SkillUp");
  static auto Key = WzResource::ui_->find(u"StatusBar.img/button:Key");
  static auto QuickSlot =
      WzResource::ui_->find(u"StatusBar.img/button:QuickSlot");
  static auto QuickSlotD =
      WzResource::ui_->find(u"StatusBar.img/button:QuickSlotD");
  static auto ChatLogMin =
      WzResource::ui_->find(u"StatusBar.img/button:ChatLogMin");
  static auto ChatLogMax =
      WzResource::ui_->find(u"StatusBar.img/button:ChatLogMax");

  std::array buttons_node = {
      CashShop,   //
      Menu,       //
      Shortcut,   //
      Mailbox,    //
      equip,      //
      Inven,      //
      Stat,       // Stat StatUp
      Skill,      // Skill SkillUp
      Key,        //
      QuickSlot,  // QuickSlot QuickSlotD
      ChatLogMin, // ChatLogMin ChatLogMax
  };
  const static std::array buttons_rect = {
      SDL_FRect{578, 38, 73, 34}, // CashShop
      SDL_FRect{652, 38, 73, 34}, // Menu
      SDL_FRect{726, 38, 73, 34}, // Shortcut
      SDL_FRect{578, 11, 22, 19}, // Mailbox
      SDL_FRect{621, 10, 28, 20}, // Equip
      SDL_FRect{651, 10, 28, 20}, // Inven
      SDL_FRect{681, 10, 28, 20}, // Stat StatUp
      SDL_FRect{711, 10, 28, 20}, // Skill SkillUp
      SDL_FRect{741, 10, 28, 20}, // Key
      SDL_FRect{771, 10, 28, 20}, // QuickSlot QuickSlotD
      SDL_FRect{539, 14, 12, 12}, // ChatLogMin ChatLogMax
  };

  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);

  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += base_x;
    pos_rect.y += base_y;
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

void StatusbarUiSystem::RenderChat() {
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  if (chat_type_.has_value()) {
    static auto TargetSelect =
        WzResource::ui_->find(u"StatusBar.img/chat/button:ChatTargetSelect");
    SDL_FRect pos_rect{6, 11, 80, 18};
    pos_rect.x += base_x;
    pos_rect.y += base_y;
    auto &mouse_pos = Window::mouse_pos_;
    // 判断按钮是否被遮挡
    auto CursorIn = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && CursorIn == Render &&
        !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
        auto pressed =
            WzResource::LoadTexture(TargetSelect->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer_, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over =
            WzResource::LoadTexture(TargetSelect->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer_, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::LoadTexture(TargetSelect->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer_, normal, nullptr, &pos_rect);
    }
    Freetype::LoadSize(12);
    Freetype::LoadAligned(true);
    Freetype::LoadColor(255, 255, 255, 255);
    auto s = LoadChatType();
    auto w = Freetype::LoadW(s);
    Freetype::DrawLine(s, pos_rect.x + 40 - w / 2, pos_rect.y);
    Freetype::LoadAligned(false);

    TextInputUiSystem::Render(chat_, 5, 5);
  } else {
    static auto chatBackgrnd = WzResource::LoadTexture(
        WzResource::ui_->find(u"StatusBar.img/chat/canvas:chatbackgrnd2"));
    SDL_FRect pos_rect = {
        base_x + 7,
        base_y + 8,
        static_cast<float>(chatBackgrnd->w),
        static_cast<float>(chatBackgrnd->h),
    };
    SDL_RenderTexture(Window::renderer_, chatBackgrnd, nullptr, &pos_rect);
  }
}

void StatusbarUiSystem::RenderBackChat() {
  auto it =
      std::ranges::find(System::render_systems_, &ChatLogUiSystem::Render);
  if (it != System::render_systems_.end()) {
    return;
  }
  if (ChatGameInstance::chats_.empty()) {
    return;
  }
  auto back = ChatGameInstance::chats_.back();
  Freetype::LoadAligned(true);
  Freetype::LoadSize(12);
  auto str = back.owner + u":" + back.text;
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  Freetype::LoadColor(255, 255, 255, 255);
  auto w = Freetype::LoadW(str);
  bool point = false;
  while (w > 476) {
    str.pop_back();
    w = Freetype::LoadW(str);
    point = true;
  }
  if (point) {
    str = str + u"...";
  }
  Freetype::DrawLine(str, base_x + 18, base_y + 10);
  Freetype::LoadAligned(false);
}

bool StatusbarUiSystem::Render() {
  RenderBackgrnd();
  RenderChat();
  RenderButton();
  RenderCharacterStat();
  RenderHmAlert();
  RenderBackChat();
  return true;
}

SDL_FPoint StatusbarUiSystem::LoadWh() { return {808, 73}; }

bool StatusbarUiSystem::CursorIn() {
  auto wh = LoadWh();
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - wh.x) / 2;
  auto base_y = (screen_h - wh.y);
  SDL_FRect pos_rect = {base_x, base_y, wh.x, wh.y};
  return SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect);
}

void StatusbarUiSystem::EventButtonCashshop() { return; }

void StatusbarUiSystem::EventButtonMenu() {
  StatusbarMenuUiSystem::menu_type_ =
      StatusbarMenuUiSystem::MenuEnums::kMenu;
  StatusbarMenuUiSystem::Toggle();
  return;
}

void StatusbarUiSystem::EventButtonShortcut() {
  StatusbarMenuUiSystem::menu_type_ =
      StatusbarMenuUiSystem::MenuEnums::kShortcut;
  StatusbarMenuUiSystem::Toggle();
  return;
}

void StatusbarUiSystem::EventButtonMailbox() { return; }

void StatusbarUiSystem::EventButtonEquip() {
  EquipUiSystem::Toggle();
  return;
}

void StatusbarUiSystem::EventButtonInven() {
  PackageUiSystem::Toggle();
  return;
}

void StatusbarUiSystem::EventButtonStat() {
  CharacterStatUiSystem::Toggle();
  return;
}

void StatusbarUiSystem::EventButtonSkill() {
  SkillUiSystem::Toggle();
  return;
}

void StatusbarUiSystem::EventButtonKeybind() {
  KeybindingUiSystem::Toggle();
  return;
}

void StatusbarUiSystem::EventButtonQuickslot() { return; }

void StatusbarUiSystem::EventButtonChatlog() {
  ChatLogUiSystem::Toggle();
  if (chat_type_.has_value()) {
    chat_type_ = std::nullopt;
  } else {
    chat_type_ = GameChatEnum::kMap;
  }
  return;
}

std::u16string StatusbarUiSystem::LoadChatType() {
  switch (chat_type_.value()) {
  case GameChatEnum::kMap: {
    return u"All";
    break;
  }
  }
  return u"";
}

bool StatusbarUiSystem::EventButton(SDL_Event *Event) {
  const static std::array buttons_rect = {
      SDL_FRect{578, 38, 73, 34}, // CashShop
      SDL_FRect{652, 38, 73, 34}, // Menu
      SDL_FRect{726, 38, 73, 34}, // Shortcut
      SDL_FRect{578, 11, 22, 19}, // Mailbox
      SDL_FRect{621, 10, 28, 20}, // Equip
      SDL_FRect{651, 10, 28, 20}, // Inven
      SDL_FRect{681, 10, 28, 20}, // Stat StatUp
      SDL_FRect{711, 10, 28, 20}, // Skill SkillUp
      SDL_FRect{741, 10, 28, 20}, // Key
      SDL_FRect{771, 10, 28, 20}, // QuickSlot QuickSlotD
      SDL_FRect{539, 14, 12, 12}, // ChatLogMin ChatLogMax
  };
  const static std::array buttons_func = {
      EventButtonCashshop,  EventButtonMenu,    EventButtonShortcut,
      EventButtonMailbox,   EventButtonEquip,   EventButtonInven,
      EventButtonStat,      EventButtonSkill,   EventButtonKeybind,
      EventButtonQuickslot, EventButtonChatlog,
  };
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto [w, h] = LoadWh();
  auto base_x = (screen_w - w) / 2;
  auto base_y = (screen_h - h);

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += base_x;
    pos_rect.y += base_y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      buttons_func[i]();
      return true;
    }
  }
  return false;
}

void StatusbarUiSystem::EventMotionBtAudio(SDL_Event *Event) {
  if (CursorGameInstance::cursor_ui != Render) {
    return;
  }
  const static std::array buttons_rect = {
      SDL_FRect{578, 38, 73, 34}, // CashShop
      SDL_FRect{652, 38, 73, 34}, // Menu
      SDL_FRect{726, 38, 73, 34}, // Shortcut
      SDL_FRect{578, 11, 22, 19}, // Mailbox
      SDL_FRect{621, 10, 28, 20}, // Equip
      SDL_FRect{651, 10, 28, 20}, // Inven
      SDL_FRect{681, 10, 28, 20}, // Stat StatUp
      SDL_FRect{711, 10, 28, 20}, // Skill SkillUp
      SDL_FRect{741, 10, 28, 20}, // Key
      SDL_FRect{771, 10, 28, 20}, // QuickSlot QuickSlotD
      SDL_FRect{539, 14, 12, 12}, // ChatLogMin ChatLogMax
  };
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto [w, h] = LoadWh();
  auto base_x = (screen_w - w) / 2;
  auto base_y = (screen_h - h);
  static int8_t bt_audio = -1;
  auto &mouse_pos = Window::mouse_pos_;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += base_x;
    pos_rect.y += base_y;
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

bool StatusbarUiSystem::Event(SDL_Event *Event) {
  auto cr = TextInputUiSystem::Event(Event, chat_);
  bool r = true;
  switch (Event->type) {
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        EventButton(Event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      std::erase(System::event_systems_, KeyboardInputSystem::Event);
      if (cr) {
        // close input
        System::event_systems_.push_back(KeyboardInputSystem::Event);
      }
      r = false;
    }
  }
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = Event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_RETURN: {
      std::erase(System::event_systems_, KeyboardInputSystem::Event);
      ChatLogUiSystem::Toggle();
      if (chat_type_.has_value()) {
        EventChatSend();
        chat_type_ = std::nullopt;
        TextInputUiSystem::Close(chat_);
        System::event_systems_.push_back(KeyboardInputSystem::Event);
      } else {
        chat_type_ = GameChatEnum::kMap;
        TextInputUiSystem::Active(chat_);
        KeyboardInputSystem::Reset();
      }
      r = false;
      break;
    }
    default: {
      break;
    }
    }
    break;
  }
  case SDL_EVENT_KEY_UP: {
    auto scan_code = Event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_RETURN: {
      break;
    }
    default: {
      break;
    }
    }
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

void StatusbarUiSystem::EventChatSend() {
  if (chat_.text == u"") {
    return;
  }
  fbs::ClientCharacterChatT cct;
  cct.map_id = SceneSystemInstance::map_id_;
  cct.payload = std::make_unique<fbs::CharacterChatT>();
  cct.payload->type = fbs::ChatEnum_MAP;
  cct.payload->payload = {chat_.text.begin(), chat_.text.end()};
  ClientRequest::send_to_host(cct);
  Reset();
}

void StatusbarUiSystem::Reset() {
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  chat_ = {
      .max_size = 80,
      .text = u"",
      .composition = {},
      .disable = false,
      .active = false,
      .r =
          SDL_Rect{
              static_cast<int>(base_x + 85),
              static_cast<int>(base_y + 6),
              420,
              25,
          },
      .font_color = {0, 0, 0, 255},
      .cur_color = {0, 0, 0, 255},
      .font_size = 12,
  };
  chat_.type.set(TextInput::kIme);
  chat_type_ = std::nullopt;
}

}  // namespace sdlms
