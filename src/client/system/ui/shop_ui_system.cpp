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

SDL_FPoint ShopUiSystem::LoadWh() { return SDL_FPoint{465, 381}; }

void ShopUiSystem::RenderBackgrnd() {
  static auto texture = WzResource::LoadTexture(
      WzResource::ui->find(u"UIShop.img/Shop/backgrnd"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
}

void ShopUiSystem::RenderItem(std::vector<GameShopItem> &items, int page,
                                 SDL_FPoint p) {
  GameShopItem *item_info = nullptr;
  const auto page_size = 6;
  for (int i = page; i < page + page_size; i++) {
    auto x = (int)pos.x + p.x;
    auto y = (int)pos.y + p.y + (i - page) * 40;
    if (i >= items.size()) {
      auto icon = WzResource::LoadTexture(
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
    if (ItemGameInstance::CheckItem(item.item->id)) {
      auto info = ItemGameInstance::LoadItemInfo(item.item->id, 0);
      texture = WzResource::LoadTexture(info->get_child(u"icon"));
      item_name = ItemGameInstance::LoadItemText(item.item->id, u"name");
    } else {
      // equip
      auto info = EquipGameInstance::LoadEquipInfo(item.item->id);
      texture = WzResource::LoadTexture(info->get_child(u"icon"));
      item_name = EquipGameInstance::LoadEquipName(item.item->id);
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
    auto item_num = ItemGameInstance::LoadItemNum(item.item);
    if (item_num > 0) {
      PackageUiSystem::RenderNumber(item_num, x, y + 24);
    }

    pos_rect.w = 202;
    pos_rect.h = 35;
    const auto &mouse_pos = Window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      item_info = &item;
    }
    // render name
    Freetype::LoadAligned(true);
    Freetype::LoadSize(12);
    Freetype::LoadColor(0, 0, 0, 255);
    x = x + 42;
    y = y - 2;
    Freetype::DrawLine(item_name, x, y);
    // render meso
    static auto meso = WzResource::LoadTexture(
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
    Freetype::DrawLine(tmp2, pos_rect.x + 15, pos_rect.y - 4);
  }

  if (item_info && CursorGameInstance::modal_overlay == Render) {
    RenderItemInfo(item_info->item);
  }
}

void ShopUiSystem::RenderPkgItems() {
  std::vector<GameShopItem> items;
  auto its = ShopGameInstance::LoadPkgItems(active_tab[1]);
  for (auto &itm : its) {
    if (itm != nullptr) {
      auto item = ShopGameInstance::LoadShopItem((*itm)->id);
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
  RenderItem(items, pages[1], {238, 129});
}

void ShopUiSystem::RenderItems() {
  if (active_tab[0] == 0) {
    auto items = shop->items;
    RenderItem(items, pages[0], {8, 129});
  } else {
    auto items = ShopGameInstance::must;
    if (items.size() < 6) {
      items.resize(6);
    }
    RenderItem(items, pages[0], {8, 129});
  }
}

void ShopUiSystem::RenderVscr() {
  const uint32_t length = 235;
  auto x = (int)pos.x + 212;
  auto y = (int)pos.y + 129;
  auto CursorIn = CursorGameInstance::cursor_ui;
  auto items_size = 0;
  if (active_tab[0] == 0) {
    auto &items = shop->items;
    items_size = items.size();
  } else {
    auto &items = ShopGameInstance::must;
    items_size = items.size();
  }
  bool top =
      CursorIn == Render && CursorGameInstance::modal_overlay == Render;
  ScrollUiSystem::RenderVscroll(x, y, pages[0], items_size, length, top, 6);
  x += 230;
  auto pkg_size = ShopGameInstance::LoadPkgItems(active_tab[1]).size();
  ScrollUiSystem::RenderVscroll(x, y, pages[1], pkg_size, length, top, 6);
  return;
}

void ShopUiSystem::RenderButton() {
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
    auto CursorIn = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && CursorIn == Render &&
        CursorGameInstance::modal_overlay == Render) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::LoadTexture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::LoadTexture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::LoadTexture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void ShopUiSystem::RenderTab() {
  const static std::array tab_pos0 = {
      SDL_FPoint{7, 96},  //
      SDL_FPoint{64, 96}, //
  };
  const static auto tab_node0 =
      WzResource::ui->find(u"UIShop.img/Shop/TabBuy");
  const static std::array active_texture0 = {
      WzResource::LoadTexture(tab_node0->find(u"enabled/0")),
      WzResource::LoadTexture(tab_node0->find(u"enabled/1")),
  };
  const static std::array disabled_texture0 = {
      WzResource::LoadTexture(tab_node0->find(u"disabled/0")),
      WzResource::LoadTexture(tab_node0->find(u"disabled/1")),
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
      WzResource::LoadTexture(tab_node1->find(u"enabled/0")),
      WzResource::LoadTexture(tab_node1->find(u"enabled/1")),
      WzResource::LoadTexture(tab_node1->find(u"enabled/2")),
      WzResource::LoadTexture(tab_node1->find(u"enabled/3")),
  };
  const static std::array disabled_texture1 = {
      WzResource::LoadTexture(tab_node1->find(u"disabled/0")),
      WzResource::LoadTexture(tab_node1->find(u"disabled/1")),
      WzResource::LoadTexture(tab_node1->find(u"disabled/2")),
      WzResource::LoadTexture(tab_node1->find(u"disabled/3")),
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

void ShopUiSystem::RenderActiveItem() {
  static auto texture = WzResource::LoadTexture(
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

void ShopUiSystem::RenderNpc() {
  if (!npc.has_value()) {
    return;
  }
  auto n = npc.value();
  n.action = u"stand";
  const auto &camera = CameraGameInstance::camera;
  n.pos.x = camera.x + pos.x + 55;
  n.pos.y = camera.y + pos.y + 76;
  n.ani_index = 0;
  NpcRenderSystem::RenderNpc(n);
}

void ShopUiSystem::RenderAvatar() {
  auto self = CharacterGameInstance::self;
  CharacterLogicSystem::RunStandAction(self);
  self.action_index = 0;
  self.flip = 0;
  self.face.action = u"default";
  self.face.index = 0;
  const auto &camera = CameraGameInstance::camera;
  self.pos.x = camera.x + (int)pos.x + 290;
  self.pos.y = camera.y + (int)pos.y + 76;
  CharacterRenderSystem::RenderCharacter(self);
}

void ShopUiSystem::RenderItemInfo(std::polymorphic<GameItem> &item) {
  auto &mouse_pos = Window::mouse_pos;
  SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
  switch (item->type) {
  case ItemEnum::kEquip: {
    auto eqp = static_cast<GameEquipItem &>(*item);
    TooltipUiSystem::RenderEquip(eqp, show_pos.x, show_pos.y);
    auto self = CharacterGameInstance::self;
    auto eqps = EquipGameInstance::LoadEquipSlot(eqp.id, self);
    if (!eqps.empty()) {
      TooltipUiSystem::RenderEquip(eqps[0], show_pos.x + 238, show_pos.y);
    }
    break;
  }
  default: {
    auto itm = static_cast<GameItem &>(*item);
    TooltipUiSystem::RenderItem(itm, show_pos.x, show_pos.y);
    break;
  }
  }
}

void ShopUiSystem::RenderMeso() {
  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(0, 0, 0, 255);
  auto meso =
      TextGameInstance::FormatWithCommas(PackageGameInstance::meso);
  std::u16string meso2 = {meso.begin(), meso.end()};
  auto w = Freetype::LoadW(meso2);
  Freetype::DrawLine(meso2, pos.x + 448 - w, pos.y + 64);
  return;
}

bool ShopUiSystem::Render() {
  RenderBackgrnd();
  RenderButton();
  RenderTab();
  RenderActiveItem();
  RenderNpc();
  RenderAvatar();
  RenderMeso();
  RenderVscr();
  RenderPkgItems();
  RenderItems();
  return true;
}

void ShopUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::Render);
  if (it != System::render_systems.end()) {
    auto wh = LoadWh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    pages = {};
    active_item = {};
    active_tab = {};
    KeyboardInputSystem::Reset();

    System::render_systems.insert(it, Render);
    System::event_systems.insert(System::event_systems.begin(), Event);
  }
}

bool ShopUiSystem::EventButton(SDL_Event *Event) {
  std::vector<void (*)()> fns;
  auto wh = LoadWh();
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

void ShopUiSystem::Close() {
  std::erase(System::render_systems, Render);
  std::erase(System::event_systems, Event);
}

bool ShopUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void ShopUiSystem::EventClose() { Close(); }

std::optional<int> ShopUiSystem::EventItemClick(SDL_Event *Event,
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

bool ShopUiSystem::EventItem(SDL_Event *Event) {
  auto item0 = EventItemClick(Event, {8, 129});
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
    bool space = PackageGameInstance::LoadEmptyIndex(itm2.item).empty();
    if (meso < itm.price) {
      NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kShopbuyNoMeso;
    } else if (space) {
      NoticeUiSystem::OpenNoSpace(itm2.item->type);
      return true;
    } else {
      NoticeUiSystem::type = (itm.item->type == ItemEnum::kEquip)
                                   ? NoticeUiSystem::NoticeEnum::kShopbuy
                                   : NoticeUiSystem::NoticeEnum::kShopbuyMul;
      NoticeUiSystem::data = &itm;
    }
    NoticeUiSystem::Open();
    return true;
  }

  auto item1 = EventItemClick(Event, {238, 129});
  if (item1.has_value()) {
    auto index = pages[1] + item1.value();
    auto items = ShopGameInstance::LoadPkgItems(active_tab[1]);
    if (index >= items.size()) {
      return true;
    }
    if (active_item[1] != index) {
      active_item[1] = index;
      active_item[0] = std::nullopt;
      return true;
    }
    auto itm = items[index];
    if ((*itm)->type == ItemEnum::kEquip) {
      NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kShopbuySell;
    } else {
      auto num = ItemGameInstance::LoadItemNum(*itm);
      auto itm_info = ItemGameInstance::LoadItemInfo((*itm)->id, 0);
      bool unitPrice = itm_info->get_child(u"unitPrice");
      if (num == 1 || unitPrice) {
        NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kShopbuySell;
      } else {
        NoticeUiSystem::type =
            NoticeUiSystem::NoticeEnum::kShopbuySellMul;
      }
    }
    NoticeUiSystem::data = itm;
    NoticeUiSystem::Open();
  }
  return false;
}

bool ShopUiSystem::EventVscr(SDL_Event *Event) {
  const uint32_t length = 235;
  auto x = (int)pos.x + 212;
  auto y = (int)pos.y + 129;

  int size = shop->items.size() - 6;
  if (active_tab[0] == 1) {
    size = ShopGameInstance::must.size() - 6;
  }
  auto CursorIn = CursorGameInstance::cursor_ui;
  bool top = CursorIn == Render;
  size = std::max(0, size);
  auto val = ScrollUiSystem::ClickVscroll(x, y, pages[0], size, length, top);
  pages[0] = val;

  size = ShopGameInstance::LoadPkgItems(active_tab[1]).size() - 6;
  x += 230;
  size = std::max(0, size);
  val = ScrollUiSystem::ClickVscroll(x, y, pages[1], size, length, top);
  pages[1] = val;
  return true;
}

bool ShopUiSystem::EventTab(SDL_Event *Event) {
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

void ShopUiSystem::EventVscrStart(SDL_Event *Event) {
  SDL_FPoint lt{212, 129};
  const uint32_t length = 235;
  if (vscr_motion[0] == false) {
    vscr_motion[0] =
        ScrollUiSystem::ClickThumb(pos.x + lt.x, pos.y + lt.y, length);
  }
  lt.x += 230;
  if (vscr_motion[1] == false) {
    vscr_motion[1] =
        ScrollUiSystem::ClickThumb(pos.x + lt.x, pos.y + lt.y, length);
  }
}

void ShopUiSystem::EventVscrEnd() { vscr_motion = {}; }

void ShopUiSystem::EventVscrMove(SDL_Event *Event) {
  auto mouse_state = Window::mouse_state;
  SDL_FPoint lt{212, 129};
  const uint32_t length = 235;
  if (vscr_motion[0]) {
    Event->button.x = pos.x + lt.x;
    Event->button.y =
        std::clamp(Event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    EventVscr(Event);
  }
  if (vscr_motion[1]) {
    lt.x += 230;
    Event->button.x = pos.x + lt.x;
    Event->button.y =
        std::clamp(Event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    EventVscr(Event);
  }
}

bool ShopUiSystem::Event(SDL_Event *Event) {
  switch (Event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = Event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      EventClose();
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
      if (CursorGameInstance::cursor_ui == Render) {
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        if (EventItem(Event)) {
          return false;
        }
        if (EventTab(Event)) {
          return false;
        }
        EventButton(Event);
        EventVscr(Event);
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