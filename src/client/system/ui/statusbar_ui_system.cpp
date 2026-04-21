#include "statusbar_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
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
  const static std::array buttons_pos = {
      SDL_FPoint{578, 38}, // CashShop
      SDL_FPoint{652, 38}, // Menu
      SDL_FPoint{726, 38}, // Shortcut
      SDL_FPoint{578, 11}, // Mailbox
      SDL_FPoint{621, 10}, // Equip
      SDL_FPoint{651, 10}, // Inven
      SDL_FPoint{681, 10}, // Stat StatUp
      SDL_FPoint{711, 10}, // Skill SkillUp
      SDL_FPoint{741, 10}, // Key
      SDL_FPoint{771, 10}, // QuickSlot QuickSlotD
      SDL_FPoint{539, 14}, // ChatLogMin ChatLogMax
  };

  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);

  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto v = buttons_pos[i];
    auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
    auto normal = wz_resource::load_texture(k->find(u"normal/0"));
    auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
    SDL_FRect pos_rect{base_x + v.x, base_y + v.y,
                       static_cast<float>(mouse_over->w),
                       static_cast<float>(mouse_over->h)};
    auto mouse_pos = window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

bool statusbar_ui_system::render() {
  render_backgrnd();
  render_button();
  return true;
}