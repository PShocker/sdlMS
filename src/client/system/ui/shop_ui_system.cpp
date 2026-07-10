#include "shop_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "notice_ui_system.h"
#include "scroll_ui_system.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_npc.h"
#include "src/client/game/game_shop.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "tooltip_ui_system.h"
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
  const game_shop_item *item_info = nullptr;
  auto &items = shop->items;
  // render shop
  const auto page_size = 6;
  for (int i = pages[0]; i < pages[0] + page_size; i++) {
    if (i >= items.size()) {
      break;
    }
    auto &item = items[i];
    SDL_Texture *texture;
    std::u16string item_name;
    if (item_game_instance::check_item(item.item->id)) {
      auto info = item_game_instance::load_item_info(item.item->id);
      texture = wz_resource::load_texture(info->get_child(u"icon"));
      item_name = item_game_instance::load_item_text(item.item->id, u"name");
    } else {
      // equip
      auto info = equip_game_instance::load_equip_info(item.item->id);
      texture = wz_resource::load_texture(info->get_child(u"icon"));
      item_name = equip_game_instance::load_equip_name(item.item->id);
    }
    SDL_FRect pos_rect;
    auto x = (int)pos.x + 8;
    auto y = (int)pos.y + 129 + (i - pages[0]) * 40;
    pos_rect.x = x + (36 - texture->w) / 2;
    pos_rect.y = y + (36 - texture->h) / 2;
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);

    // render item info
    pos_rect.x = x;
    pos_rect.y = y;
    pos_rect.w = 36;
    pos_rect.h = 36;
    const auto &mouse_pos = window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      item_info = &item;
    }

    // render name
    freetype::load_aligned(true);
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
  if (item_info && !cursor_game_instance::modal_overlay) {
    render_item_info(*item_info->item);
  }
}

void shop_ui_system::render_vscr() {
  const uint32_t length = 235;
  auto x = (int)pos.x + 212;
  auto y = (int)pos.y + 129;
  auto cursor_in = cursor_game_instance::cursor_ui;
  auto &items = shop->items;
  bool top = cursor_in == render && !cursor_game_instance::modal_overlay;
  scroll_ui_system::render_vscroll(x, y, pages[0], items.size(), length, top);

  x += 230;
  scroll_ui_system::render_vscroll(x, y, pages[0], items.size(), length, top);
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
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        cursor_game_instance::modal_overlay == render) {
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
      SDL_FPoint{7, 96},  //
      SDL_FPoint{64, 96}, //
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
  static auto texture = wz_resource::load_texture(
      wz_resource::ui->find(u"PlayerShop.img/Store/ItemEntry/selected"));
  if (active_item[0].has_value()) {
    auto x = (int)pos.x + 45;
    auto y = (int)pos.y + 129 + (active_item[0].value() - pages[0]) * 40;
    SDL_FRect pos_rect;
    pos_rect.x = x;
    pos_rect.y = y;
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
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

void shop_ui_system::render_item_info(game_item &item) {
  auto &mouse_pos = window::mouse_pos;
  SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
  switch (item.type) {
  case item_enum::equip: {
    auto eqp = static_cast<game_equip_item &>(item);
    tooltip_ui_system::render_equip(eqp, show_pos.x, show_pos.y);
    auto self = character_game_instance::self;
    auto eqps = equip_game_instance::load_equip_slot(eqp, self);
    if (!eqps.empty()) {
      tooltip_ui_system::render_equip(eqps[0], show_pos.x + 238, show_pos.y);
    }
    break;
  }
  default: {
    tooltip_ui_system::render_item(item, show_pos.x, show_pos.y);
    break;
  }
  }
}

bool shop_ui_system::render() {
  render_backgrnd();
  render_button();
  render_tab();
  render_active_item();
  render_npc();
  render_self();
  render_vscr();
  render_items();
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

bool shop_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void shop_ui_system::event_close() { close(); }

bool shop_ui_system::event_item(SDL_Event *event) {
  auto x = (int)pos.x + 8;
  auto y = (int)pos.y + 129;
  const auto &mouse_pos = window::mouse_pos;
  SDL_FRect pos_rect;
  pos_rect.x = x;
  pos_rect.y = y;
  pos_rect.w = 202;
  pos_rect.h = 240;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    auto &items = shop->items;
    int item = pages[0] + (mouse_pos.y - y) / 40;
    if (item < items.size()) {
      if (active_item[0] == item) {
        auto meso = package_game_instance::meso;
        const auto &itm = items[item];
        if (meso < itm.price) {
          notice_ui_system::type =
              notice_ui_system::notice_enum::shopbuy_no_meso;
        }
        notice_ui_system::open();
      } else {
        active_item[0] = item;
      }
    }
    return true;
  }
  return false;
}

bool shop_ui_system::event(SDL_Event *event) {
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      event_close();
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
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        if (event_item(event)) {
          return false;
        }
        event_button(event);
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

  return false;
}