#include "statusbar_ui_system.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "skill_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <flat_map>
#include <format>
#include <string>
#include <vector>

void statusbar_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/backgrnd2"));
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - backgrnd->w) / 2;
  auto base_y = (screen_h - backgrnd->h);
  SDL_FRect pos_rect = {base_x, base_y, static_cast<float>(backgrnd->w),
                        static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);

  static auto backgrnd2 = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/canvas:backgrnd1"));
  pos_rect = {base_x + 7, base_y + 9, static_cast<float>(backgrnd2->w),
              static_cast<float>(backgrnd2->h)};
  SDL_RenderTexture(window::renderer, backgrnd2, nullptr, &pos_rect);
  static auto gaugeLabel = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/canvas:gaugeLabel"));
  pos_rect = {base_x + 223, base_y + 40, static_cast<float>(gaugeLabel->w),
              static_cast<float>(gaugeLabel->h)};
  SDL_RenderTexture(window::renderer, gaugeLabel, nullptr, &pos_rect);
}

void statusbar_ui_system::render_gauge_text() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/backgrnd2"));

  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - backgrnd->w) / 2;
  auto base_y = (screen_h - backgrnd->h);
  static auto num_node = wz_resource::ui->find(u"StatusBar.img/gauge/number");
  std::array<int32_t, 2> a = {character_stat_game_instance::hp_point,
                              character_stat_game_instance::mp_point};
  std::array<int32_t, 2> a2 = {character_stat_game_instance::hp_point_max,
                               character_stat_game_instance::mp_point_max};
  std::array<SDL_FPoint, 2> a3 = {
      SDL_FPoint{245, 42},
      SDL_FPoint{356, 42},
  };
  SDL_FRect pos_rect;
  for (auto i : {0, 1}) {
    auto l_bracket = wz_resource::load_texture(num_node->get_child(u"["));
    auto r_bracket = wz_resource::load_texture(num_node->get_child(u"]"));
    pos_rect = {base_x + a3[i].x, base_y + a3[i].y,
                static_cast<float>(l_bracket->w),
                static_cast<float>(l_bracket->h)};
    SDL_RenderTexture(window::renderer, l_bracket, nullptr, &pos_rect);
    int32_t w = 6;
    a[i] = std::max(a[i], 0);
    auto num = std::to_string(a[i]);
    for (auto c : num) {
      pos_rect.x = base_x + w + a3[i].x;
      auto texture =
          wz_resource::load_texture(num_node->get_child(std::string{c}));
      pos_rect.w = texture->w;
      pos_rect.h = texture->h;
      SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
      w += 6;
    }
    auto texture = wz_resource::load_texture(num_node->get_child(u"slash"));
    pos_rect.x = base_x + w + a3[i].x;
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
    w += 6;

    num = std::to_string(a2[i]);
    for (auto c : num) {
      pos_rect.x = base_x + w + a3[i].x;
      auto texture =
          wz_resource::load_texture(num_node->get_child(std::string{c}));
      pos_rect.w = texture->w;
      pos_rect.h = texture->h;
      SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
      w += 6;
    }

    pos_rect.x = base_x + w + a3[i].x;
    pos_rect.w = r_bracket->w;
    pos_rect.h = r_bracket->h;
    SDL_RenderTexture(window::renderer, r_bracket, nullptr, &pos_rect);
  }
  SDL_FPoint a4{473, 42};
  auto self_exp = character_stat_game_instance::exp_point;
  auto self_max_exp = character_stat_game_instance::exp_point_max;
  float self_exp_percent = (float)self_exp / self_max_exp;
  auto num = std::to_string(self_exp);
  int32_t w = 0;
  for (auto c : num) {
    pos_rect.x = base_x + w + a4.x;
    auto texture =
        wz_resource::load_texture(num_node->get_child(std::string{c}));
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
    w += 6;
  }
  auto l_bracket = wz_resource::load_texture(num_node->get_child(u"["));
  auto r_bracket = wz_resource::load_texture(num_node->get_child(u"]"));

  pos_rect.x += 8;
  SDL_RenderTexture(window::renderer, l_bracket, nullptr, &pos_rect);

  auto exp_percent_num = std::format("{:.2f}", self_exp_percent * 100);
  for (auto c : exp_percent_num) {
    auto texture =
        wz_resource::load_texture(num_node->get_child(std::string{c}));
    pos_rect.x += 6;
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
  auto percent_texture = wz_resource::load_texture(num_node->get_child(u"%"));
  pos_rect.x += 6;
  pos_rect.w = percent_texture->w;
  pos_rect.h = percent_texture->h;
  SDL_RenderTexture(window::renderer, percent_texture, nullptr, &pos_rect);

  pos_rect.x += 8;
  pos_rect.w = r_bracket->w;
  pos_rect.h = r_bracket->h;
  SDL_RenderTexture(window::renderer, r_bracket, nullptr, &pos_rect);
}

void statusbar_ui_system::render_character_stat() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/backgrnd2"));

  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - backgrnd->w) / 2;
  auto base_y = (screen_h - backgrnd->h);

  static auto gaugeHp = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/gauge/layer:gaugeHp"));

  static auto gray = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/gauge/canvas:gaugeBack"));

  auto self_hp = character_stat_game_instance::hp_point;
  auto self_max_hp = character_stat_game_instance::hp_point_max;
  auto self_hp_percent = (float)self_hp / self_max_hp;
  static auto last_hp_percent = self_hp_percent;
  SDL_FRect pos_rect;
  pos_rect = {
      base_x + 224,
      base_y + 56,
      static_cast<float>(gaugeHp->w),
      static_cast<float>(gray->h),
  };
  SDL_RenderTexture(window::renderer, gray, nullptr, &pos_rect);

  last_hp_percent = std::lerp(last_hp_percent, self_hp_percent, 0.1);
  SDL_FRect src_rect{
      0,
      0,
      static_cast<float>(gaugeHp->w * last_hp_percent),
      static_cast<float>(gaugeHp->h),
  };
  pos_rect = {
      base_x + 224,
      base_y + 56,
      static_cast<float>(src_rect.w),
      static_cast<float>(gaugeHp->h),
  };
  SDL_RenderTexture(window::renderer, gaugeHp, &src_rect, &pos_rect);

  auto self_mp = character_stat_game_instance::mp_point;
  auto self_max_mp = character_stat_game_instance::mp_point_max;
  auto self_mp_percent = (float)self_mp / self_max_mp;
  static auto gaugeMp = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/gauge/layer:gaugeMp"));
  pos_rect = {
      base_x + 332,
      base_y + 56,
      static_cast<float>(gaugeMp->w),
      static_cast<float>(gray->h),
  };
  SDL_RenderTexture(window::renderer, gray, nullptr, &pos_rect);

  src_rect = {
      0,
      0,
      static_cast<float>(gaugeMp->w * self_mp_percent),
      static_cast<float>(gaugeMp->h),
  };

  pos_rect = {base_x + 332, base_y + 56, static_cast<float>(src_rect.w),
              static_cast<float>(gaugeMp->h)};
  SDL_RenderTexture(window::renderer, gaugeMp, &src_rect, &pos_rect);

  static auto gaugeExp = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/gauge/layer:gaugeExp"));
  auto self_exp = character_stat_game_instance::exp_point;
  auto self_max_exp = character_stat_game_instance::exp_point_max;
  auto self_exp_percent = (float)self_exp / self_max_exp;

  pos_rect = {
      base_x + 445,
      base_y + 56,
      static_cast<float>(gaugeExp->w),
      static_cast<float>(gray->h),
  };
  SDL_RenderTexture(window::renderer, gray, nullptr, &pos_rect);

  src_rect = {
      0,
      0,
      static_cast<float>(gaugeExp->w * self_exp_percent),
      static_cast<float>(gaugeExp->h),
  };

  pos_rect = {base_x + 445, base_y + 56, static_cast<float>(src_rect.w),
              static_cast<float>(gaugeExp->h)};
  SDL_RenderTexture(window::renderer, gaugeExp, nullptr, &pos_rect);

  static auto gaugeCover = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/layer:gaugeCover"));
  pos_rect = {base_x + 223, base_y + 55, static_cast<float>(gaugeCover->w),
              static_cast<float>(gaugeCover->h)};
  SDL_RenderTexture(window::renderer, gaugeCover, nullptr, &pos_rect);

  // job
  auto job_node = wz_resource::ms->get_root()->find(
      u"String.img/Job/" + character_game_instance::self.job);
  auto job_name = static_cast<wz::Property<std::u16string> *>(job_node)->get();
  freetype::load_size(12);
  freetype::load_color(255, 255, 255, 255);
  freetype::load_aligned(true);
  freetype::draw_line(job_name, base_x + 88, base_y + 36);
  // name
  const auto &self = character_game_instance::self;
  auto &self_name = self.nametags[0].text;
  freetype::load_size(12);
  freetype::load_color(255, 255, 255, 255);
  freetype::draw_line(self_name, base_x + 88, base_y + 52);
  freetype::load_aligned(false);

  // level
  static auto lvNumber = wz_resource::ui->find(u"StatusBar.img/lvNumber");
  auto level = self.level;
  auto level_str = std::to_string(level);
  pos_rect.x = base_x + 44;
  pos_rect.y = base_y + 49;
  int32_t w = 0;
  for (auto c : level_str) {
    auto t = wz_resource::load_texture(lvNumber->get_child(std::string{c}));
    pos_rect.x += w;
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    w = 12;
  }

  render_gauge_text();
}

void statusbar_ui_system::render_button() {
  static auto CashShop =
      wz_resource::ui->find(u"StatusBar.img/button:CashShop");
  static auto Menu = wz_resource::ui->find(u"StatusBar.img/button:Menu");
  static auto Shortcut =
      wz_resource::ui->find(u"StatusBar.img/button:Shortcut");
  static auto Mailbox = wz_resource::ui->find(u"StatusBar.img/button:Mailbox");
  static auto Equip = wz_resource::ui->find(u"StatusBar.img/button:Equip");
  static auto Inven = wz_resource::ui->find(u"StatusBar.img/button:Inven");
  static auto Stat = wz_resource::ui->find(u"StatusBar.img/button:Stat");
  static auto StatUp = wz_resource::ui->find(u"StatusBar.img/button:StatUp");
  static auto Skill = wz_resource::ui->find(u"StatusBar.img/button:Skill");
  static auto SkillUp = wz_resource::ui->find(u"StatusBar.img/button:SkillUp");
  static auto Key = wz_resource::ui->find(u"StatusBar.img/button:Key");
  static auto QuickSlot =
      wz_resource::ui->find(u"StatusBar.img/button:QuickSlot");
  static auto QuickSlotD =
      wz_resource::ui->find(u"StatusBar.img/button:QuickSlotD");
  static auto ChatLogMin =
      wz_resource::ui->find(u"StatusBar.img/button:ChatLogMin");
  static auto ChatLogMax =
      wz_resource::ui->find(u"StatusBar.img/button:ChatLogMax");

  std::array buttons_node = {
      CashShop,   //
      Menu,       //
      Shortcut,   //
      Mailbox,    //
      Equip,      //
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

  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);

  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += base_x;
    pos_rect.y += base_y;
    auto &mouse_pos = window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = wz_resource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void statusbar_ui_system::render_quickSlot() {
  const static std::flat_map<SDL_Scancode, SDL_Texture *> textures = {
      {SDL_SCANCODE_PAGEDOWN, wz_resource::load_texture(wz_resource::ui->find(
                                  u"QuickSlot.img/key/81"))},
      {SDL_SCANCODE_END, wz_resource::load_texture(
                             wz_resource::ui->find(u"QuickSlot.img/key/79"))},
      {SDL_SCANCODE_PAGEUP, wz_resource::load_texture(wz_resource::ui->find(
                                u"QuickSlot.img/key/73"))},
      {SDL_SCANCODE_HOME, wz_resource::load_texture(
                              wz_resource::ui->find(u"QuickSlot.img/key/71"))},
      {SDL_SCANCODE_INSERT, wz_resource::load_texture(wz_resource::ui->find(
                                u"QuickSlot.img/key/82"))},
      {SDL_SCANCODE_DELETE, wz_resource::load_texture(wz_resource::ui->find(
                                u"QuickSlot.img/key/83"))},
      {SDL_SCANCODE_LCTRL, wz_resource::load_texture(
                               wz_resource::ui->find(u"QuickSlot.img/key/29"))},
      {SDL_SCANCODE_LSHIFT, wz_resource::load_texture(wz_resource::ui->find(
                                u"QuickSlot.img/key/42"))},
  };
  switch (quickSlot) {
  case quick_slot::hide: {
    break;
  }
  case quick_slot::two: {
    static auto q = wz_resource::load_texture(
        wz_resource::ui->find(u"QuickSlot.img/backgrnd"));
    auto screen_w = camera_game_instance::camera.w;
    auto screen_h = camera_game_instance::camera.h;
    auto base_x = (screen_w - 808) / 2;
    auto base_y = (screen_h - 73);
    SDL_FRect p{base_x + 654, base_y - 107, static_cast<float>(q->w),
                static_cast<float>(q->h)};
    SDL_RenderTexture(window::renderer, q, nullptr, &p);

    std::vector<SDL_Texture *> t = {
        textures.at(SDL_SCANCODE_LSHIFT), textures.at(SDL_SCANCODE_INSERT),
        textures.at(SDL_SCANCODE_HOME),   textures.at(SDL_SCANCODE_PAGEUP),
        textures.at(SDL_SCANCODE_LCTRL),  textures.at(SDL_SCANCODE_DELETE),
        textures.at(SDL_SCANCODE_END),    textures.at(SDL_SCANCODE_PAGEDOWN),
    };
    std::vector<SDL_FPoint> r = {
        SDL_FPoint{p.x + 9, p.y + 10},
        SDL_FPoint{p.x + 44, p.y + 10},
        SDL_FPoint{p.x + 79, p.y + 10},
        SDL_FPoint{p.x + 114, p.y + 10},
        // 2row
        SDL_FPoint{p.x + 9, p.y + 44},
        SDL_FPoint{p.x + 44, p.y + 44},
        SDL_FPoint{p.x + 79, p.y + 44},
        SDL_FPoint{p.x + 114, p.y + 44},
    };
    for (int i = 0; i < r.size(); i++) {
      auto texture = t[i];
      SDL_FRect pos_rect{
          r[i].x,
          r[i].y + 34,
          static_cast<float>(texture->w),
          static_cast<float>(texture->h),
      };
      SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
    }

    break;
  }
  case quick_slot::three: {
    break;
  }
  }
}

void statusbar_ui_system::render_chat() {
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  if (chat.has_value()) {
    static auto TargetSelect =
        wz_resource::ui->find(u"StatusBar.img/chat/button:ChatTargetSelect");
    SDL_FRect pos_rect{6, 11, 80, 18};
    pos_rect.x += base_x;
    pos_rect.y += base_y;
    auto &mouse_pos = window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed =
            wz_resource::load_texture(TargetSelect->find(u"pressed/0"));
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over =
            wz_resource::load_texture(TargetSelect->find(u"mouseOver/0"));
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = wz_resource::load_texture(TargetSelect->find(u"normal/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
    freetype::load_size(12);
    freetype::load_aligned(true);
    freetype::load_color(255, 255, 255, 255);
    auto s = load_chat_type_str();
    auto w = freetype::load_w(s);
    freetype::draw_line(s, pos_rect.x + 40 - w / 2, pos_rect.y);
    freetype::load_aligned(false);

    static auto chatbackgrnd1 = wz_resource::load_texture(
        wz_resource::ui->find(u"StatusBar.img/chat/canvas:chatbackgrnd1"));
    pos_rect.x = base_x;
    pos_rect.y = base_y - 71;
    pos_rect.w = chatbackgrnd1->w;
    pos_rect.h = 76;

    SDL_RenderTexture(window::renderer, chatbackgrnd1, nullptr, &pos_rect);
  } else {
    static auto chatBackgrnd = wz_resource::load_texture(
        wz_resource::ui->find(u"StatusBar.img/chat/canvas:chatbackgrnd2"));
    SDL_FRect pos_rect = {
        base_x + 7,
        base_y + 8,
        static_cast<float>(chatBackgrnd->w),
        static_cast<float>(chatBackgrnd->h),
    };
    SDL_RenderTexture(window::renderer, chatBackgrnd, nullptr, &pos_rect);
  }
}

bool statusbar_ui_system::render() {
  render_backgrnd();
  render_chat();
  render_button();
  render_quickSlot();
  render_character_stat();
  return true;
}

SDL_FPoint statusbar_ui_system::load_wh() { return {808, 73}; }

bool statusbar_ui_system::cursor_in() {
  auto wh = load_wh();
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - wh.x) / 2;
  auto base_y = (screen_h - wh.y);
  SDL_FRect pos_rect = {base_x, base_y, wh.x, wh.y};
  return SDL_PointInRectFloat(&window::mouse_pos, &pos_rect);
}

void statusbar_ui_system::event_button_cashshop() { return; }

void statusbar_ui_system::event_button_menu() { return; }

void statusbar_ui_system::event_button_shortcut() { return; }

void statusbar_ui_system::event_button_mailbox() { return; }

void statusbar_ui_system::event_button_equip() { return; }

void statusbar_ui_system::event_button_inven() {
  package_ui_system::toggle();
  return;
}

void statusbar_ui_system::event_button_stat() { return; }

void statusbar_ui_system::event_button_skill() {
  skill_ui_system::toggle();
  return;
}

void statusbar_ui_system::event_button_keybind() { return; }

void statusbar_ui_system::event_button_quickslot() { return; }

void statusbar_ui_system::event_button_chatlog() { return; }

std::u16string statusbar_ui_system::load_chat_type_str() {
  switch (chat_type.value()) {
  case all: {
    return u"All";
    break;
  }
  }
  return u"";
}

bool statusbar_ui_system::event_button(SDL_Event *event) {
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
      event_button_cashshop,  event_button_menu,    event_button_shortcut,
      event_button_mailbox,   event_button_equip,   event_button_inven,
      event_button_stat,      event_button_skill,   event_button_keybind,
      event_button_quickslot, event_button_chatlog,
  };
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto [w, h] = load_wh();
  auto base_x = (screen_w - w) / 2;
  auto base_y = (screen_h - h);

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += base_x;
    pos_rect.y += base_y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      buttons_func[i]();
      return true;
    }
  }
  return false;
}

bool statusbar_ui_system::event_click_quickslot(SDL_Event *event) {
  if (cursor_game_instance::cursor_hand_net.has_value()) {
    return false; 
  }
  switch (quickSlot) {
  case quick_slot::hide: {
    break;
  }
  case quick_slot::two: {
    static auto q = wz_resource::load_texture(
        wz_resource::ui->find(u"QuickSlot.img/backgrnd"));
    auto screen_w = camera_game_instance::camera.w;
    auto screen_h = camera_game_instance::camera.h;
    auto base_x = (screen_w - 808) / 2;
    auto base_y = (screen_h - 73);
    SDL_FRect p{base_x + 654, base_y - 107, static_cast<float>(q->w),
                static_cast<float>(q->h)};
    std::vector<SDL_Scancode> t = {
        SDL_SCANCODE_LSHIFT, SDL_SCANCODE_INSERT,   SDL_SCANCODE_HOME,
        SDL_SCANCODE_PAGEUP, SDL_SCANCODE_LCTRL,    SDL_SCANCODE_DELETE,
        SDL_SCANCODE_END,    SDL_SCANCODE_PAGEDOWN,
    };
    std::vector<SDL_FPoint> r = {
        SDL_FPoint{p.x + 9, p.y + 10},
        SDL_FPoint{p.x + 44, p.y + 10},
        SDL_FPoint{p.x + 79, p.y + 10},
        SDL_FPoint{p.x + 114, p.y + 10},
        // 2row
        SDL_FPoint{p.x + 9, p.y + 44},
        SDL_FPoint{p.x + 44, p.y + 44},
        SDL_FPoint{p.x + 79, p.y + 44},
        SDL_FPoint{p.x + 114, p.y + 44},
    };
    for (int i = 0; i < r.size(); i++) {
      SDL_FRect pos_rect{
          r[i].x,
          r[i].y + 34,
          static_cast<float>(32),
          static_cast<float>(32),
      };
      if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
        if (cursor_game_instance::cursor_hand.has_value()) {
          auto &cursor_hand = cursor_game_instance::cursor_hand.value();
          auto scan_code = t[i];
          switch (cursor_hand.type) {
          case cursor_game_instance::equipment: {
            break;
          }
          case cursor_game_instance::package: {
            break;
          }
          case cursor_game_instance::skill: {
            auto ski_id = std::format("{:07d}", cursor_hand.sub_val);
            auto &key_data = keyboard_game_instance::data;
            key_data[scan_code] = {
                .type = "skill",
                .val = ski_id,
            };
            break;
          }
          case cursor_game_instance::keybind: {
            break;
          }
          }
        } else {
          auto &key_data = keyboard_game_instance::data;
          auto scan_code = t[i];
          if (key_data.contains(scan_code)) {
            auto &input = key_data.at(scan_code);
            if (input.type == "skill") {
              auto ski_id = std::stoi(input.val);
              cursor_game_instance::cursor_hand = {
                  .type = cursor_game_instance::skill,
                  .val = static_cast<uint32_t>(ski_id),
              };
              return true;
            }
          }
        }
        return true;
      }
    }

    break;
  }
  case quick_slot::three: {
    break;
  }
  }
  return false;
}

bool statusbar_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        event_button(event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_BACKSPACE: {
      break;
    }
    default: {
      break;
    }
    }
    break;
  }
  case SDL_EVENT_KEY_UP: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_RETURN: {
      break;
    }
    default: {
      break;
    }
    }
  }
  default: {
    break;
  }
  }

  return r;
}