#include "shop_ui_system.h"
#include "SDL3/SDL_render.h"
#include "scroll_ui_system.h"
#include "src/client/game/game_npc.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <algorithm>
#include <string>

SDL_FPoint shop_ui_system::load_wh() { return SDL_FPoint{465, 381}; }

void shop_ui_system::render_backgrnd() {
  static auto texture = wz_resource::load_texture(
      wz_resource::ui->find(u"UIShop.img/Shop/backgrnd"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

void shop_ui_system::render_items() {
  if (!shop.has_value()) {
    return;
  }
  auto items = shop->items;
  // render shop
  const auto page_size = 6;
  for (int i = pages[0]; i < pages[0] + page_size; i++) {
    if (i >= items.size()) {
      break;
    }
    auto item = items[i];
    SDL_Texture *texture;
    std::u16string item_name;
    if (item_game_instance::check_item(item.itemId)) {
      auto info = item_game_instance::load_item_info(item.itemId);
      texture = wz_resource::load_texture(info->get_child(u"icon"));
      item_name = item_game_instance::load_item_text(item.itemId, u"name");
    } else {
      // equip
      auto info = equip_game_instance::load_equip_info(item.itemId);
      texture = wz_resource::load_texture(info->get_child(u"icon"));
      item_name = equip_game_instance::load_equip_name(item.itemId);
    }
    SDL_FRect pos_rect;
    auto x = (int)pos.x + 8;
    auto y = (int)pos.y + 129 + (i - pages[0]) * 40;
    pos_rect.x = x + (36 - texture->w) / 2;
    pos_rect.y = y + (36 - texture->h) / 2;
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);

    // render name
    freetype::load_size(12);
    freetype::load_color(0, 0, 0, 255);
    x = x + 42;
    y = y - 2;
    freetype::draw_line(item_name, x, y);
    // render meso
    static auto meso = wz_resource::load_texture(
        wz_resource::ui->find(u"UIShop.img/Shop/meso"));
    pos_rect.x = x;
    pos_rect.y = y + 23;
    pos_rect.w = meso->w;
    pos_rect.h = meso->h;
    SDL_RenderTexture(window::renderer, meso, nullptr, &pos_rect);
    auto tmp = std::to_string(item.price);
    std::u16string tmp2{tmp.begin(), tmp.end()};
    auto n = wz_resource::ms->get_root()->find(u"String.img/Shop/mesos");
    tmp2 = tmp2 + u" " + static_cast<wz::Property<std::u16string> *>(n)->get();
    freetype::draw_line(tmp2, pos_rect.x + 15, pos_rect.y - 4);
  }
}

void shop_ui_system::render_vscr() {
  const SDL_FPoint lt{188, 51};
  const uint32_t length = 202;
  auto size = 96 / 5;
  auto cursor_in = cursor_game_instance::cursor_ui;
  scroll_ui_system::render_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y,
                                   pages[0], size, length, true);
  return;
}

void shop_ui_system::render_button() {
  const static std::array buttons_node = {
      wz_resource::ui->find(u"UIShop.img/Shop/BtBuy"),
      wz_resource::ui->find(u"UIShop.img/Shop/BtSell"),
      wz_resource::ui->find(u"UIShop.img/Shop/BtExit"),
  };
  const std::array buttons_rect = {
      SDL_FRect{144, 34, 80, 18}, //
      SDL_FRect{374, 34, 80, 18}, //
      SDL_FRect{144, 14, 80, 18}, //
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
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

void shop_ui_system::render_tab() {
  const static std::array tab_pos0 = {
      SDL_FPoint{237, 96}, //
      SDL_FPoint{281, 96}, //
  };
  const static auto tab_node0 =
      wz_resource::ui->find(u"UIShop.img/Shop/TabBuy");
  const static std::array active_texture0 = {
      wz_resource::load_texture(tab_node0->find(u"enabled/0")),
      wz_resource::load_texture(tab_node0->find(u"enabled/1")),
  };
  const static std::array disabled_texture0 = {
      wz_resource::load_texture(tab_node0->find(u"disabled/0")),
      wz_resource::load_texture(tab_node0->find(u"disabled/1")),
  };
  for (uint8_t i = 0; i < tab_pos0.size(); i++) {
    SDL_Texture *t =
        active_tab[0] == i ? active_texture0[i] : disabled_texture0[i];
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos0[i].x)),
                       static_cast<float>(int(pos.y + tab_pos0[i].y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  }

  //
  const static std::array tab_pos1 = {
      SDL_FPoint{237, 96}, //
      SDL_FPoint{281, 96}, //
      SDL_FPoint{325, 96}, //
      SDL_FPoint{369, 96}, //
  };
  const static auto tab_node1 =
      wz_resource::ui->find(u"UIShop.img/Shop/TabSell");
  const static std::array active_texture1 = {
      wz_resource::load_texture(tab_node1->find(u"enabled/0")),
      wz_resource::load_texture(tab_node1->find(u"enabled/1")),
      wz_resource::load_texture(tab_node1->find(u"enabled/2")),
      wz_resource::load_texture(tab_node1->find(u"enabled/3")),
  };
  const static std::array disabled_texture1 = {
      wz_resource::load_texture(tab_node1->find(u"disabled/0")),
      wz_resource::load_texture(tab_node1->find(u"disabled/1")),
      wz_resource::load_texture(tab_node1->find(u"disabled/2")),
      wz_resource::load_texture(tab_node1->find(u"disabled/3")),
  };
  for (uint8_t i = 0; i < tab_pos1.size(); i++) {
    SDL_Texture *t =
        active_tab[1] == i ? active_texture1[i] : disabled_texture1[i];
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos1[i].x)),
                       static_cast<float>(int(pos.y + tab_pos1[i].y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  }
}

void shop_ui_system::render_active_item() {
  
}

void shop_ui_system::render_npc() {
  if (!npc.has_value()) {
    return;
  }
  auto n = npc.value();
  n.action = u"stand";
  const auto &camera = camera_game_instance::camera;
  n.pos.x = camera.x + pos.x + 55;
  n.pos.y = camera.y + pos.y + 76;
  n.ani_index = 0;
  npc_render_system::render_npc(n);
}

void shop_ui_system::render_self() {
  auto self = character_game_instance::self;
  character_logic_system::run_stand_action(self);
  self.action_index = 0;
  self.flip = 0;
  self.face.action = u"default";
  self.face.index = 0;
  const auto &camera = camera_game_instance::camera;
  self.pos.x = camera.x + (int)pos.x + 290;
  self.pos.y = camera.y + (int)pos.y + 76;
  character_render_system::render_character(self);
}

bool shop_ui_system::render() {
  render_backgrnd();
  render_button();
  render_tab();
  render_items();
  render_npc();
  render_self();
  return true;
}

void shop_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    pages = {};
    active_item = {};
    active_tab = {};
    keyboard_input_system::reset();

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

bool shop_ui_system::event_button(SDL_Event *event) {
  std::vector<void (*)()> fns;
  auto wh = load_wh();
  std::vector<SDL_FRect> buttons_rect = {

  };
  fns = {

  };

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

void shop_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void shop_ui_system::event_close() { close(); }

bool shop_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      event_close();
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
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        r = !event_button(event);
      }
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