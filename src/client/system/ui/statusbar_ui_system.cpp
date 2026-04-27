#include "statusbar_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "skill_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

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

  static auto gaugeCover = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/layer:gaugeCover"));
  pos_rect = {base_x + 223, base_y + 55, static_cast<float>(gaugeCover->w),
              static_cast<float>(gaugeCover->h)};
  SDL_RenderTexture(window::renderer, gaugeCover, nullptr, &pos_rect);

  static auto chatBackgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"StatusBar.img/chat/canvas:chatbackgrnd2"));
  pos_rect = {base_x + 7, base_y + 8, static_cast<float>(chatBackgrnd->w),
              static_cast<float>(chatBackgrnd->h)};
  SDL_RenderTexture(window::renderer, chatBackgrnd, nullptr, &pos_rect);
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
  if (quickSlot) {
    static auto q = wz_resource::load_texture(
        wz_resource::ui->find(u"QuickSlot.img/backgrnd"));
    auto screen_w = camera_game_instance::camera.w;
    auto screen_h = camera_game_instance::camera.h;
    auto base_x = (screen_w - 808) / 2;
    auto base_y = (screen_h - 73);
    SDL_FRect p{base_x + 654, base_y - 107, static_cast<float>(q->w),
                static_cast<float>(q->h)};
    SDL_RenderTexture(window::renderer, q, nullptr, &p);
  }
}

bool statusbar_ui_system::render() {
  render_backgrnd();
  render_button();
  render_quickSlot();
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
  default: {
    break;
  }
  }

  return r;
}