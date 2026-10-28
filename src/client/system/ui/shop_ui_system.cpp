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
#include "src/client/game_instance/text_game_instance.h"
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

SDL_FPoint ShopUiSystem::load_wh() { return SDL_FPoint{465, 381}; }

void ShopUiSystem::render_backgrnd() {
  static auto texture = WzResource::load_texture(
      WzResource::ui->find(u"UIShop.img/Shop/backgrnd"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
}

void ShopUiSystem::render_item(std::vector<GameShopItem> &items, int page,
                                 SDL_FPoint p) {
  GameShopItem *item_info = nullptr;
  const auto page_size = 6;
  for (int i = page; i < page + page_size; i++) {
    auto x = (int)pos.x + p.x;
    auto y = (int)pos.y + p.y + (i - page) * 40;
    if (i >= items.size()) {
      auto icon = WzResource::load_texture(
          WzResource::ui->find(u"Item.img/canvas:disabled"));
      SDL_FRect pos_rect{
          std::floor(x),
          std::floor(y),
          static_cast<float>(icon->w),
          static_cast<float>(icon->h),
      };
      SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);
      continue;
    }
    auto &item = items[i];
    if (item.item->id.empty()) {
      continue;
    }
    SDL_Texture *texture;
    std::u16string item_name;
    if (ItemGameInstance::check_item(item.item->id)) {
      auto info = ItemGameInstance::load_item_info(item.item->id, 0);
      texture = WzResource::load_texture(info->get_child(u"icon"));
      item_name = ItemGameInstance::load_item_text(item.item->id, u"name");
    } else {
      // equip
      auto info = EquipGameInstance::load_equip_info(item.item->id);
      texture = WzResource::load_texture(info->get_child(u"icon"));
      item_name = EquipGameInstance::load_equip_name(item.item->id);
    }
    SDL_FRect pos_rect;

    pos_rect.x = x + (36 - texture->w) / 2;
    pos_rect.y = y + (36 - texture->h) / 2;
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);

    // render item info
    pos_rect.x = x;
    pos_rect.y = y;
    pos_rect.w = 36;
    pos_rect.h = 36;
    // render num
    auto item_num = ItemGameInstance::load_item_num(item.item);
    if (item_num > 0) {
      PackageUiSystem::render_number(item_num, x, y + 24);
    }

    pos_rect.w = 202;
    pos_rect.h = 35;
    const auto &mouse_pos = Window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      item_info = &item;
    }
    // render name
    Freetype::load_aligned(true);
    Freetype::load_size(12);
    Freetype::load_color(0, 0, 0, 255);
    x = x + 42;
    y = y - 2;
    Freetype::draw_line(item_name, x, y);
    // render meso
    static auto meso = WzResource::load_texture(
        WzResource::ui->find(u"UIShop.img/Shop/meso"));
    pos_rect.x = x;
    pos_rect.y = y + 23;
    pos_rect.w = meso->w;
    pos_rect.h = meso->h;
    SDL_RenderTexture(Window::renderer, meso, nullptr, &pos_rect);
    auto tmp = std::to_string(item.price);
    std::u16string tmp2{tmp.begin(), tmp.end()};
    auto n = WzResource::ms->get_root()->find(u"String.img/Shop/mesos");
    tmp2 = tmp2 + u" " + static_cast<wz::Property<std::u16string> *>(n)->get();
    Freetype::draw_line(tmp2, pos_rect.x + 15, pos_rect.y - 4);
  }

  if (item_info && CursorGameInstance::modal_overlay == render) {
    render_item_info(item_info->item);
  }
}

void ShopUiSystem::render_pkg_items() {
  std::vector<GameShopItem> items;
  auto its = ShopGameInstance::load_pkg_items(active_tab[1]);
  for (auto &itm : its) {
    if (itm != nullptr) {
      auto item = ShopGameInstance::load_shop_item((*itm)->id);
      item.item = *itm;
      items.push_back(std::move(item));
    } else {
      GameShopItem item;
      items.push_back(item);
    }
  }
  auto &r = PackageGameInstance::data[active_tab[1]];
  if (items.size() < 6) {
    auto min = std::min(items.size() + 6, r.size());
    items.resize(min);
  }
  render_item(items, pages[1], {238, 129});
}

void ShopUiSystem::render_items() {
  if (active_tab[0] == 0) {
    auto items = shop->items;
    render_item(items, pages[0], {8, 129});
  } else {
    auto items = ShopGameInstance::must;
    if (items.size() < 6) {
      items.resize(6);
    }
    render_item(items, pages[0], {8, 129});
  }
}

void ShopUiSystem::render_vscr() {
  const uint32_t length = 235;
  auto x = (int)pos.x + 212;
  auto y = (int)pos.y + 129;
  auto cursor_in = CursorGameInstance::cursor_ui;
  auto items_size = 0;
  if (active_tab[0] == 0) {
    auto &items = shop->items;
    items_size = items.size();
  } else {
    auto &items = ShopGameInstance::must;
    items_size = items.size();
  }
  bool top =
      cursor_in == render && CursorGameInstance::modal_overlay == render;
  ScrollUiSystem::render_vscroll(x, y, pages[0], items_size, length, top, 6);
  x += 230;
  auto pkg_size = ShopGameInstance::load_pkg_items(active_tab[1]).size();
  ScrollUiSystem::render_vscroll(x, y, pages[1], pkg_size, length, top, 6);
  return;
}

void ShopUiSystem::render_button() {
  const static std::array buttons_node = {
      WzResource::ui->find(u"UIShop.img/Shop/BtBuy"),
      WzResource::ui->find(u"UIShop.img/Shop/BtSell"),
      WzResource::ui->find(u"UIShop.img/Shop/BtExit"),
  };
  const std::array buttons_rect = {
      SDL_FRect{154, 34, 68, 16}, //
      SDL_FRect{384, 34, 68, 16}, //
      SDL_FRect{154, 14, 68, 16}, //
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        CursorGameInstance::modal_overlay == render) {
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

void ShopUiSystem::render_tab() {
  const static std::array tab_pos0 = {
      SDL_FPoint{7, 96},  //
      SDL_FPoint{64, 96}, //
  };
  const static auto tab_node0 =
      WzResource::ui->find(u"UIShop.img/Shop/TabBuy");
  const static std::array active_texture0 = {
      WzResource::load_texture(tab_node0->find(u"enabled/0")),
      WzResource::load_texture(tab_node0->find(u"enabled/1")),
  };
  const static std::array disabled_texture0 = {
      WzResource::load_texture(tab_node0->find(u"disabled/0")),
      WzResource::load_texture(tab_node0->find(u"disabled/1")),
  };
  for (uint8_t i = 0; i < tab_pos0.size(); i++) {
    SDL_Texture *t =
        active_tab[0] == i ? active_texture0[i] : disabled_texture0[i];
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos0[i].x)),
                       static_cast<float>(int(pos.y + tab_pos0[i].y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  }

  //
  const static std::array tab_pos1 = {
      SDL_FPoint{237, 96}, //
      SDL_FPoint{281, 96}, //
      SDL_FPoint{325, 96}, //
      SDL_FPoint{369, 96}, //
  };
  const static auto tab_node1 =
      WzResource::ui->find(u"UIShop.img/Shop/TabSell");
  const static std::array active_texture1 = {
      WzResource::load_texture(tab_node1->find(u"enabled/0")),
      WzResource::load_texture(tab_node1->find(u"enabled/1")),
      WzResource::load_texture(tab_node1->find(u"enabled/2")),
      WzResource::load_texture(tab_node1->find(u"enabled/3")),
  };
  const static std::array disabled_texture1 = {
      WzResource::load_texture(tab_node1->find(u"disabled/0")),
      WzResource::load_texture(tab_node1->find(u"disabled/1")),
      WzResource::load_texture(tab_node1->find(u"disabled/2")),
      WzResource::load_texture(tab_node1->find(u"disabled/3")),
  };
  for (uint8_t i = 0; i < tab_pos1.size(); i++) {
    SDL_Texture *t =
        active_tab[1] == i ? active_texture1[i] : disabled_texture1[i];
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos1[i].x)),
                       static_cast<float>(int(pos.y + tab_pos1[i].y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  }
}

void ShopUiSystem::render_active_item() {
  static auto texture = WzResource::load_texture(
      WzResource::ui->find(u"PlayerShop.img/Store/ItemEntry/selected"));
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
      SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
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
      SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
    }
  }
}

void ShopUiSystem::render_npc() {
  if (!npc.has_value()) {
    return;
  }
  auto n = npc.value();
  n.action = u"stand";
  const auto &camera = CameraGameInstance::camera;
  n.pos.x = camera.x + pos.x + 55;
  n.pos.y = camera.y + pos.y + 76;
  n.ani_index = 0;
  NpcRenderSystem::render_npc(n);
}

void ShopUiSystem::render_avatar() {
  auto self = CharacterGameInstance::self;
  CharacterLogicSystem::run_stand_action(self);
  self.action_index = 0;
  self.flip = 0;
  self.face.action = u"default";
  self.face.index = 0;
  const auto &camera = CameraGameInstance::camera;
  self.pos.x = camera.x + (int)pos.x + 290;
  self.pos.y = camera.y + (int)pos.y + 76;
  CharacterRenderSystem::render_character(self);
}

void ShopUiSystem::render_item_info(std::polymorphic<GameItem> &item) {
  auto &mouse_pos = Window::mouse_pos;
  SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
  switch (item->type) {
  case ItemEnum::equip: {
    auto eqp = static_cast<GameEquipItem &>(*item);
    TooltipUiSystem::render_equip(eqp, show_pos.x, show_pos.y);
    auto self = CharacterGameInstance::self;
    auto eqps = EquipGameInstance::load_equip_slot(eqp.id, self);
    if (!eqps.empty()) {
      TooltipUiSystem::render_equip(eqps[0], show_pos.x + 238, show_pos.y);
    }
    break;
  }
  default: {
    auto itm = static_cast<GameItem &>(*item);
    TooltipUiSystem::render_item(itm, show_pos.x, show_pos.y);
    break;
  }
  }
}

void ShopUiSystem::render_meso() {
  Freetype::load_size(12);
  Freetype::load_aligned(true);
  Freetype::load_color(0, 0, 0, 255);
  auto meso =
      TextGameInstance::format_with_commas(PackageGameInstance::meso);
  std::u16string meso2 = {meso.begin(), meso.end()};
  auto w = Freetype::load_w(meso2);
  Freetype::draw_line(meso2, pos.x + 448 - w, pos.y + 64);
  return;
}

bool ShopUiSystem::render() {
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

void ShopUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    pages = {};
    active_item = {};
    active_tab = {};
    KeyboardInputSystem::reset();

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

bool ShopUiSystem::event_button(SDL_Event *event) {
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
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

void ShopUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
}

bool ShopUiSystem::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void ShopUiSystem::event_close() { close(); }

std::optional<int> ShopUiSystem::event_item_click(SDL_Event *event,
                                                    SDL_FPoint p) {
  auto x = (int)pos.x + p.x;
  auto y = (int)pos.y + p.y;
  const auto &mouse_pos = Window::mouse_pos;
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

bool ShopUiSystem::event_item(SDL_Event *event) {
  auto item0 = event_item_click(event, {8, 129});
  if (item0.has_value()) {
    const std::vector<GameShopItem> *items =
        (active_tab[0] == 0) ? &shop->items : &ShopGameInstance::must;
    auto index = pages[0] + item0.value();
    if (index >= items->size()) {
      return true;
    }
    if (active_item[0] != index) {
      active_item[0] = index;
      active_item[1] = std::nullopt;
      return true;
    }
    auto meso = PackageGameInstance::meso;
    const auto &itm = items->at(index);
    auto itm2 = items->at(index);
    bool space = PackageGameInstance::load_empty_index(itm2.item).empty();
    if (meso < itm.price) {
      NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::shopbuy_no_meso;
    } else if (space) {
      NoticeUiSystem::open_no_space(itm2.item->type);
      return true;
    } else {
      NoticeUiSystem::type = (itm.item->type == ItemEnum::equip)
                                   ? NoticeUiSystem::NoticeEnum::shopbuy
                                   : NoticeUiSystem::NoticeEnum::shopbuy_mul;
      NoticeUiSystem::data = &itm;
    }
    NoticeUiSystem::open();
    return true;
  }

  auto item1 = event_item_click(event, {238, 129});
  if (item1.has_value()) {
    auto index = pages[1] + item1.value();
    auto items = ShopGameInstance::load_pkg_items(active_tab[1]);
    if (index >= items.size()) {
      return true;
    }
    if (active_item[1] != index) {
      active_item[1] = index;
      active_item[0] = std::nullopt;
      return true;
    }
    auto itm = items[index];
    if ((*itm)->type == ItemEnum::equip) {
      NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::shopbuy_sell;
    } else {
      auto num = ItemGameInstance::load_item_num(*itm);
      auto itm_info = ItemGameInstance::load_item_info((*itm)->id, 0);
      bool unitPrice = itm_info->get_child(u"unitPrice");
      if (num == 1 || unitPrice) {
        NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::shopbuy_sell;
      } else {
        NoticeUiSystem::type =
            NoticeUiSystem::NoticeEnum::shopbuy_sell_mul;
      }
    }
    NoticeUiSystem::data = itm;
    NoticeUiSystem::open();
  }
  return false;
}

bool ShopUiSystem::event_vscr(SDL_Event *event) {
  const uint32_t length = 235;
  auto x = (int)pos.x + 212;
  auto y = (int)pos.y + 129;

  int size = shop->items.size() - 6;
  if (active_tab[0] == 1) {
    size = ShopGameInstance::must.size() - 6;
  }
  auto cursor_in = CursorGameInstance::cursor_ui;
  bool top = cursor_in == render;
  size = std::max(0, size);
  auto val = ScrollUiSystem::click_vscroll(x, y, pages[0], size, length, top);
  pages[0] = val;

  size = ShopGameInstance::load_pkg_items(active_tab[1]).size() - 6;
  x += 230;
  size = std::max(0, size);
  val = ScrollUiSystem::click_vscroll(x, y, pages[1], size, length, top);
  pages[1] = val;
  return true;
}

bool ShopUiSystem::event_tab(SDL_Event *event) {
  const auto &mouse_pos = Window::mouse_pos;

  // 第一组标签页（商店分类）
  const static std::array tab_pos0 = {
      SDL_FPoint{7, 96},
      SDL_FPoint{64, 96},
  };
  for (uint8_t i = 0; i < tab_pos0.size(); ++i) {
    SDL_FRect pos_rect{
        static_cast<float>(int(pos.x + tab_pos0[i].x)),
        static_cast<float>(int(pos.y + tab_pos0[i].y)),
        57,
        21,
    };
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

void ShopUiSystem::event_vscr_start(SDL_Event *event) {
  SDL_FPoint lt{212, 129};
  const uint32_t length = 235;
  if (vscr_motion[0] == false) {
    vscr_motion[0] =
        ScrollUiSystem::click_thumb(pos.x + lt.x, pos.y + lt.y, length);
  }
  lt.x += 230;
  if (vscr_motion[1] == false) {
    vscr_motion[1] =
        ScrollUiSystem::click_thumb(pos.x + lt.x, pos.y + lt.y, length);
  }
}

void ShopUiSystem::event_vscr_end() { vscr_motion = {}; }

void ShopUiSystem::event_vscr_move(SDL_Event *event) {
  auto mouse_state = Window::mouse_state;
  SDL_FPoint lt{212, 129};
  const uint32_t length = 235;
  if (vscr_motion[0]) {
    event->button.x = pos.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    event_vscr(event);
  }
  if (vscr_motion[1]) {
    lt.x += 230;
    event->button.x = pos.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    event_vscr(event);
  }
}

bool ShopUiSystem::event(SDL_Event *event) {
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
      if (CursorGameInstance::cursor_ui == render) {
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
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