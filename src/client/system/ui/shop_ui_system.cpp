#include "shop_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "notice_ui_system.h"
#include "package_ui_system.h"
#include "scroll_ui_system.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_npc.h"
#include "src/client/game/game_shop.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/shop_game_instance.h"
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
#include <optional>
#include <string>
#include <utility>
#include <vector>

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

void shop_ui_system::render_item(std::vector<game_shop_item> &items, int page,
                                 SDL_FPoint p) {
  game_shop_item *item_info = nullptr;
  const auto page_size = 6;
  for (int i = page; i < page + page_size; i++) {
    if (i >= items.size()) {
      break;
    }
    auto &item = items[i];
    SDL_Texture *texture;
    std::u16string item_name;
    if (item_game_instance::check_item(item.item->id)) {
      auto info = item_game_instance::load_item_info(item.item->id, 0);
      texture = wz_resource::load_texture(info->get_child(u"icon"));
      item_name = item_game_instance::load_item_text(item.item->id, u"name");
    } else {
      // equip
      auto info = equip_game_instance::load_equip_info(item.item->id);
      texture = wz_resource::load_texture(info->get_child(u"icon"));
      item_name = equip_game_instance::load_equip_name(item.item->id);
    }
    SDL_FRect pos_rect;
    auto x = (int)pos.x + p.x;
    auto y = (int)pos.y + p.y + (i - page) * 40;
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
    // render num
    auto item_num = item_game_instance::load_item_num(item.item);
    if (item_num > 0) {
      package_ui_system::render_number(item_num, x, y + 24);
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
  if (item_info && cursor_game_instance::modal_overlay == render) {
    render_item_info(item_info->item);
  }
}

std::vector<std::polymorphic<game_item> *> shop_ui_system::load_pkg_items() {
  std::vector<std::polymorphic<game_item> *> items;
  auto &r = package_game_instance::data[active_tab[1]];
  for (auto &itm : r) {
    if (!itm->id.empty()) {
      items.push_back(&itm);
    }
  }
  return items;
}

void shop_ui_system::render_pkg_items() {
  std::vector<game_shop_item> items;
  auto its = load_pkg_items();
  for (auto &itm : its) {
    auto item = shop_game_instance::load_shop_item((*itm)->id);
    item.item = *itm;
    items.push_back(std::move(item));
  }
  render_item(items, pages[1], {238, 129});
}

void shop_ui_system::render_items() {
  if (active_tab[0] == 0) {
    auto items = shop->items;
    render_item(items, pages[0], {8, 129});
  } else {
    auto items = must;
    render_item(items, pages[0], {8, 129});
  }
}

void shop_ui_system::render_vscr() {
  const uint32_t length = 235;
  auto x = (int)pos.x + 212;
  auto y = (int)pos.y + 129;
  auto cursor_in = cursor_game_instance::cursor_ui;
  auto &items = shop->items;
  bool top =
      cursor_in == render && cursor_game_instance::modal_overlay == render;
  auto itm_size = items.size();
  scroll_ui_system::render_vscroll(x, y, pages[0], itm_size, length, top, 6);
  x += 230;
  auto pkg_size = load_pkg_items().size();
  scroll_ui_system::render_vscroll(x, y, pages[1], pkg_size, length, top, 6);
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
    auto index = (active_item[0].value() - pages[0]);
    if (index >= 0 && index < 6) {
      auto y = (int)pos.y + 129 + index * 40;
      SDL_FRect pos_rect;
      pos_rect.x = x;
      pos_rect.y = y;
      pos_rect.w = texture->w;
      pos_rect.h = texture->h;
      SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
    }
  }
  if (active_item[1].has_value()) {
    auto x = (int)pos.x + 275;
    auto index = (active_item[1].value() - pages[1]);
    if (index >= 0 && index < 6) {
      auto y = (int)pos.y + 129 + index * 40;
      SDL_FRect pos_rect;
      pos_rect.x = x;
      pos_rect.y = y;
      pos_rect.w = texture->w;
      pos_rect.h = texture->h;
      SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
    }
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

void shop_ui_system::render_avatar() {
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

void shop_ui_system::render_item_info(std::polymorphic<game_item> &item) {
  auto &mouse_pos = window::mouse_pos;
  SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
  switch (item->type) {
  case item_enum::equip: {
    auto eqp = static_cast<game_equip_item &>(*item);
    tooltip_ui_system::render_equip(eqp, show_pos.x, show_pos.y);
    auto self = character_game_instance::self;
    auto eqps = equip_game_instance::load_equip_slot(eqp, self);
    if (!eqps.empty()) {
      tooltip_ui_system::render_equip(eqps[0], show_pos.x + 238, show_pos.y);
    }
    break;
  }
  default: {
    auto itm = static_cast<game_item &>(*item);
    tooltip_ui_system::render_item(itm, show_pos.x, show_pos.y);
    break;
  }
  }
}

void shop_ui_system::render_meso() {
  freetype::load_size(12);
  freetype::load_aligned(true);
  freetype::load_color(0, 0, 0, 255);
  auto meso = std::to_string(package_game_instance::meso);
  std::u16string meso2 = {meso.begin(), meso.end()};
  auto w = freetype::load_w(meso2);
  freetype::draw_line(meso2, pos.x + 448 - w, pos.y + 64);
  return;
}

bool shop_ui_system::render() {
  render_backgrnd();
  render_button();
  render_tab();
  render_active_item();
  render_npc();
  render_avatar();
  render_meso();
  render_vscr();
  render_pkg_items();
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

std::optional<int> shop_ui_system::event_item_click(SDL_Event *event,
                                                    SDL_FPoint p) {
  auto x = (int)pos.x + p.x;
  auto y = (int)pos.y + p.y;
  const auto &mouse_pos = window::mouse_pos;
  SDL_FRect pos_rect;
  pos_rect.x = x;
  pos_rect.y = y;
  pos_rect.w = 202;
  pos_rect.h = 238;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    return (mouse_pos.y - y) / 40;
  }
  return std::nullopt;
}

bool shop_ui_system::event_item(SDL_Event *event) {
  auto item0 = event_item_click(event, {8, 129});
  if (item0.has_value()) {
    const std::vector<game_shop_item> *items =
        (active_tab[0] == 0) ? &shop->items : &must;
    auto index = pages[0] + item0.value();
    if (index >= items->size()) {
      return true;
    }
    if (active_item[0] != index) {
      active_item[0] = index;
      active_item[1] = std::nullopt;
      return true;
    }
    auto meso = package_game_instance::meso;
    const auto &itm = items->at(index);
    auto itm2 = items->at(index);
    bool space = package_ui_system::load_b_index(itm2.item).empty();
    if (meso < itm.price) {
      notice_ui_system::type = notice_ui_system::notice_enum::shopbuy_no_meso;
    } else if (space) {
      notice_ui_system::type = notice_ui_system::notice_enum::shopbuy_no_space;
    } else {
      notice_ui_system::type = (itm.item->type == item_enum::equip)
                                   ? notice_ui_system::notice_enum::shopbuy
                                   : notice_ui_system::notice_enum::shopbuy_mul;
      notice_ui_system::data = &itm;
    }
    notice_ui_system::open();
    return true;
  }

  auto item1 = event_item_click(event, {238, 129});
  if (item1.has_value()) {
    auto index = pages[1] + item1.value();
    auto items = load_pkg_items();
    if (index >= items.size()) {
      return true;
    }
    if (active_item[1] != index) {
      active_item[1] = index;
      active_item[0] = std::nullopt;
      return true;
    }
    auto itm = items[index];
    notice_ui_system::type =
        ((*itm)->type == item_enum::equip)
            ? notice_ui_system::notice_enum::shopbuy_sell
            : notice_ui_system::notice_enum::shopbuy_sell_mul;
    notice_ui_system::open();
    notice_ui_system::data = itm;
  }
  return false;
}

bool shop_ui_system::event_vscr(SDL_Event *event) {
  const uint32_t length = 235;
  auto x = (int)pos.x + 212;
  auto y = (int)pos.y + 129;

  int size = shop->items.size() - 6;
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top = cursor_in == render;
  size = std::max(0, size);
  auto val = scroll_ui_system::click_vscroll(x, y, pages[0], size, length, top);
  pages[0] = val;

  size = load_pkg_items().size() - 6;
  x += 230;
  size = std::max(0, size);
  val = scroll_ui_system::click_vscroll(x, y, pages[1], size, length, top);
  pages[1] = val;
  return true;
}

bool shop_ui_system::event_tab(SDL_Event *event) {
  const auto &mouse_pos = window::mouse_pos;

  // 第一组标签页（商店分类）
  const static std::array tab_pos0 = {
      SDL_FPoint{7, 96},
      SDL_FPoint{64, 96},
  };
  for (uint8_t i = 0; i < tab_pos0.size(); ++i) {
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos0[i].x)),
                       static_cast<float>(int(pos.y + tab_pos0[i].y)), 57, 21};
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      active_tab[0] = i;
      active_item[0] = std::nullopt;
      pages[0] = 0;
      return true;
    }
  }

  // 第二组标签页（背包分类）
  const static std::array tab_pos1 = {
      SDL_FPoint{237, 96},
      SDL_FPoint{281, 96},
      SDL_FPoint{325, 96},
      SDL_FPoint{369, 96},
  };
  for (uint8_t i = 0; i < tab_pos1.size(); ++i) {
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos1[i].x)),
                       static_cast<float>(int(pos.y + tab_pos1[i].y)), 44, 21};
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      active_tab[1] = i;
      active_item[1] = std::nullopt;
      pages[1] = 0;
      return true;
    }
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
        if (event_tab(event)) {
          return false;
        }
        event_button(event);
        event_vscr(event);
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

void shop_ui_system::add_item_num(std::polymorphic<game_item> &item, int num) {
  switch (item->type) {
  case item_enum::consume: {
    auto &consume = static_cast<game_consume_item &>(*item);
    consume.num += num;
    break;
  }
  case item_enum::etc: {
    auto &etc = static_cast<game_etc_item &>(*item);
    etc.num += num;
    break;
  }
  default: {
    break;
  }
  }
  return;
}

void shop_ui_system::add_item_slot(std::polymorphic<game_item> &item, int i) {
  auto num = item_game_instance::load_item_num(item);
  auto &p = package_game_instance::data[(int)item->type];
  auto slot_max = item_game_instance::load_slot_max(item->id);
  auto &itm = p.at(i);
  itm->id = item->id;
  auto itm_num = item_game_instance::load_item_num(itm);
  num = std::min((int)num + itm_num, slot_max);
  add_item_num(itm, num - itm_num);
  dec_item_num(item, num - itm_num);
  return;
}

bool shop_ui_system::add_item(std::polymorphic<game_item> &item) {
  auto b = package_ui_system::load_b_index(item);
  if (b.empty()) {
    return false;
  }
  auto &p = package_game_instance::data[(int)item->type];
  switch (item->type) {
  case item_enum::consume:
  case item_enum::etc: {
    for (auto i : b) {
      add_item_slot(item, i);
    }
    break;
  }
  default: {
    p[b[0]] = (item);
    break;
  }
  }
  return true;
}

bool shop_ui_system::add_must_item(std::polymorphic<game_item> &item) {
  auto add_num = item_game_instance::load_item_num(item);
  game_shop_item gst = shop_game_instance::load_shop_item((*item).id);
  gst.item = item;
  auto slot_max = 1;
  if (item_game_instance::check_item(item->id)) {
    slot_max = item_game_instance::load_slot_max(item->id);
  }
  switch (item->type) {
  case item_enum::consume:
  case item_enum::etc: {
    for (auto &shop : must) {
      auto itm_id = shop.item->id;
      if (item->id == itm_id) {
        auto itm_num = item_game_instance::load_item_num(shop.item);
        auto num = std::min((int)add_num + itm_num, slot_max);
        add_item_num(shop.item, num - itm_num);
        add_num = add_num - (num - itm_num);
      }
    }
    while (add_num > 0) {
      auto num = std::min((int)add_num, slot_max);
      auto itm_num = item_game_instance::load_item_num(gst.item);
      add_item_num(gst.item, (num - itm_num));
      must.push_back(gst);
      add_num = add_num - (num);
    }
    break;
  }
  default: {
    must.push_back(gst);
    break;
  }
  }
  return true;
}

void shop_ui_system::dec_item_num(std::polymorphic<game_item> &item, int num) {
  switch (item->type) {
  case item_enum::equip: {
    item = std::polymorphic<game_item>(game_equip_item{});
    break;
  }
  case item_enum::consume: {
    auto &consume = static_cast<game_consume_item &>(*item);
    consume.num = consume.num - num;
    if (consume.num == 0) {
      item = std::polymorphic<game_item>(game_consume_item{});
    }
    break;
  }
  case item_enum::etc: {
    auto &etc = static_cast<game_etc_item &>(*item);
    etc.num = etc.num - num;
    if (etc.num == 0) {
      item = std::polymorphic<game_item>(game_etc_item{});
    }
    break;
  }
  case item_enum::install: {
    item = std::polymorphic<game_item>(game_install_item{});
    break;
  }
  case item_enum::cash: {
    item = std::polymorphic<game_item>(game_cash_item{});
    break;
  }
  default: {
    break;
  }
  }
  return;
}