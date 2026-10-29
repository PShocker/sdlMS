#include "src/client/system/ui/shop_ui_system.h"

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
#include "src/client/system/ui/component/button.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/component/scroll.h"
#include "src/client/system/ui/component/tab.h"
#include "src/client/system/ui/tooltip_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Property.h>

#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace sdlms {

SDL_FPoint ShopUiSystem::LoadWh() {
  return SDL_FPoint{465, 381};
}

void ShopUiSystem::RenderBackgrnd() {
  static auto texture = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIShop.img/Shop/backgrnd"));
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

void ShopUiSystem::RenderItem(std::vector<GameShopItem>& items, int page,
                              SDL_FPoint p) {
  GameShopItem* item_info = nullptr;
  const auto page_size = 6;
  for (int i = page; i < page + page_size; i++) {
    auto x = (int)pos_.x + p.x;
    auto y = (int)pos_.y + p.y + (i - page) * 40;
    if (i >= items.size()) {
      auto icon = WzResource::LoadTexture(
          WzResource::ui_->find(u"Item.img/canvas:disabled"));
      SDL_FRect pos_rect{
          std::floor(x),
          std::floor(y),
          static_cast<float>(icon->w),
          static_cast<float>(icon->h),
      };
      SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
      continue;
    }
    auto& item = items[i];
    if (item.item->id.empty()) {
      continue;
    }
    SDL_Texture* texture;
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
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);

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
    const auto& mouse_pos = Window::mouse_pos_;
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
    static auto meso =
        WzResource::LoadTexture(WzResource::ui_->find(u"UIShop.img/Shop/meso"));
    pos_rect.x = x;
    pos_rect.y = y + 23;
    pos_rect.w = meso->w;
    pos_rect.h = meso->h;
    SDL_RenderTexture(Window::renderer_, meso, nullptr, &pos_rect);
    auto tmp = std::to_string(item.price);
    std::u16string tmp2{tmp.begin(), tmp.end()};
    auto n = WzResource::ms_->get_root()->find(u"String.img/Shop/mesos");
    tmp2 = tmp2 + u" " + static_cast<wz::Property<std::u16string>*>(n)->get();
    Freetype::DrawLine(tmp2, pos_rect.x + 15, pos_rect.y - 4);
  }

  if (item_info && CursorGameInstance::modal_overlay_ == Render) {
    RenderItemInfo(item_info->item);
  }
}

void ShopUiSystem::RenderPkgItems() {
  std::vector<GameShopItem> items;
  auto its = ShopGameInstance::LoadPkgItems(active_tab_[1]);
  for (auto& itm : its) {
    if (itm != nullptr) {
      auto item = ShopGameInstance::LoadShopItem((*itm)->id);
      item.item = *itm;
      items.push_back(std::move(item));
    } else {
      GameShopItem item;
      items.push_back(item);
    }
  }
  auto& r = PackageGameInstance::data_[active_tab_[1]];
  if (items.size() < 6) {
    auto min = std::min(items.size() + 6, r.size());
    items.resize(min);
  }
  RenderItem(items, pages_[1], {238, 129});
}

void ShopUiSystem::RenderItems() {
  if (active_tab_[0] == 0) {
    auto items = shop_->items;
    RenderItem(items, pages_[0], {8, 129});
  } else {
    auto items = ShopGameInstance::must_;
    if (items.size() < 6) {
      items.resize(6);
    }
    RenderItem(items, pages_[0], {8, 129});
  }
}

void ShopUiSystem::RenderVscr() {
  const uint32_t length = 235;
  auto x = (int)pos_.x + 212;
  auto y = (int)pos_.y + 129;
  auto cursor_in = CursorGameInstance::cursor_ui_;
  auto items_size = 0;
  if (active_tab_[0] == 0) {
    auto& items = shop_->items;
    items_size = items.size();
  } else {
    auto& items = ShopGameInstance::must_;
    items_size = items.size();
  }
  bool top =
      cursor_in == Render && CursorGameInstance::modal_overlay_ == Render;
  Scroll scroll(pages_[0], items_size, length, 6);
  scroll.Render(x, y, top);
  x += 230;
  auto pkg_size = ShopGameInstance::LoadPkgItems(active_tab_[1]).size();
  Scroll scroll1(pages_[1], pkg_size, length, 6);
  scroll1.Render(x, y, top);
  return;
}

void ShopUiSystem::RenderButton() {
  for (Button& button : GetComponent()) {
    button.Render();
  }
}

std::vector<Button>& ShopUiSystem::GetComponent() {
  components_.clear();
  const static std::array buttons_node = {
      WzResource::ui_->find(u"UIShop.img/Shop/BtBuy"),
      WzResource::ui_->find(u"UIShop.img/Shop/BtSell"),
      WzResource::ui_->find(u"UIShop.img/Shop/BtExit"),
  };
  const std::array buttons_rect = {
      SDL_FRect{154, 34, 68, 16},  //
      SDL_FRect{384, 34, 68, 16},  //
      SDL_FRect{154, 14, 68, 16},  //
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x += (int)pos_.x;
    rect.y += (int)pos_.y;
    components_.push_back(
        Button{.node = buttons_node[i],
               .rect = rect,
               .enable = true,
               .focused = CursorGameInstance::cursor_ui_ == Render &&
                          !CursorGameInstance::modal_overlay_});
  }
  return components_;
}


void ShopUiSystem::RenderTab() {
  const static auto tab_node0 =
      WzResource::ui_->find(u"UIShop.img/Shop/TabBuy");
  std::vector<SDL_FRect> rects0;
  for (uint8_t i = 0; i < 2; i++) {
    rects0.push_back(SDL_FRect{static_cast<float>(int(pos_.x + 7 + i * 57)),
                               static_cast<float>(int(pos_.y + 96)), 57, 21});
  }
  Tab tab0{tab_node0, u"enabled", u"disabled", rects0,
           static_cast<uint8_t>(active_tab_[0])};
  tab0.Render();

  const static auto tab_node1 =
      WzResource::ui_->find(u"UIShop.img/Shop/TabSell");
  std::vector<SDL_FRect> rects1;
  for (uint8_t i = 0; i < 4; i++) {
    rects1.push_back(SDL_FRect{static_cast<float>(int(pos_.x + 237 + i * 44)),
                               static_cast<float>(int(pos_.y + 96)), 44, 21});
  }
  Tab tab1{tab_node1, u"enabled", u"disabled", rects1,
           static_cast<uint8_t>(active_tab_[1])};
  tab1.Render();
}

void ShopUiSystem::RenderActiveItem() {
  static auto texture = WzResource::LoadTexture(
      WzResource::ui_->find(u"PlayerShop.img/Store/ItemEntry/selected"));
  if (active_item_[0].has_value()) {
    auto x = (int)pos_.x + 45;
    auto index = (active_item_[0].value() - pages_[0]);
    if (index >= 0 && index < 6) {
      auto y = (int)pos_.y + 129 + index * 40;
      SDL_FRect pos_rect;
      pos_rect.x = x;
      pos_rect.y = y;
      pos_rect.w = texture->w;
      pos_rect.h = texture->h;
      SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
    }
  }
  if (active_item_[1].has_value()) {
    auto x = (int)pos_.x + 275;
    auto index = (active_item_[1].value() - pages_[1]);
    if (index >= 0 && index < 6) {
      auto y = (int)pos_.y + 129 + index * 40;
      SDL_FRect pos_rect;
      pos_rect.x = x;
      pos_rect.y = y;
      pos_rect.w = texture->w;
      pos_rect.h = texture->h;
      SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
    }
  }
}

void ShopUiSystem::RenderNpc() {
  if (!npc_.has_value()) {
    return;
  }
  auto n = npc_.value();
  n.action = u"stand";
  const auto& camera = CameraGameInstance::camera_;
  n.pos.x = camera.x + pos_.x + 55;
  n.pos.y = camera.y + pos_.y + 76;
  n.ani_index = 0;
  NpcRenderSystem::RenderNpc(n);
}

void ShopUiSystem::RenderAvatar() {
  auto self = CharacterGameInstance::self_;
  CharacterLogicSystem::RunStandAction(self);
  self.action_index = 0;
  self.flip = 0;
  self.face.action = u"default";
  self.face.index = 0;
  const auto& camera = CameraGameInstance::camera_;
  self.pos.x = camera.x + (int)pos_.x + 290;
  self.pos.y = camera.y + (int)pos_.y + 76;
  CharacterRenderSystem::RenderCharacter(self);
}

void ShopUiSystem::RenderItemInfo(std::polymorphic<GameItem>& item) {
  auto& mouse_pos = Window::mouse_pos_;
  SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
  switch (item->type) {
    case ItemEnum::kEquip: {
      auto eqp = static_cast<GameEquipItem&>(*item);
      TooltipUiSystem::RenderEquip(eqp, show_pos.x, show_pos.y);
      auto self = CharacterGameInstance::self_;
      auto eqps = EquipGameInstance::LoadEquipSlot(eqp.id, self);
      if (!eqps.empty()) {
        TooltipUiSystem::RenderEquip(eqps[0], show_pos.x + 238, show_pos.y);
      }
      break;
    }
    default: {
      auto itm = static_cast<GameItem&>(*item);
      TooltipUiSystem::RenderItem(itm, show_pos.x, show_pos.y);
      break;
    }
  }
}

void ShopUiSystem::RenderMeso() {
  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(0, 0, 0, 255);
  auto meso = TextGameInstance::FormatWithCommas(PackageGameInstance::meso_);
  std::u16string meso2 = {meso.begin(), meso.end()};
  auto w = Freetype::LoadW(meso2);
  Freetype::DrawLine(meso2, pos_.x + 448 - w, pos_.y + 64);
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
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    pages_ = {};
    active_item_ = {};
    active_tab_ = {};
    KeyboardInputSystem::Reset();

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

bool ShopUiSystem::EventButton(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), false);
}

void ShopUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

bool ShopUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void ShopUiSystem::EventClose() {
  Close();
}

std::optional<int> ShopUiSystem::EventItemClick(SDL_Event* event,
                                                SDL_FPoint p) {
  auto x = (int)pos_.x + p.x;
  auto y = (int)pos_.y + p.y;
  const auto& mouse_pos = Window::mouse_pos_;
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

bool ShopUiSystem::EventItem(SDL_Event* event) {
  auto item0 = EventItemClick(event, {8, 129});
  if (item0.has_value()) {
    const std::vector<GameShopItem>* items =
        (active_tab_[0] == 0) ? &shop_->items : &ShopGameInstance::must_;
    auto index = pages_[0] + item0.value();
    if (index >= items->size()) {
      return true;
    }
    if (active_item_[0] != index) {
      active_item_[0] = index;
      active_item_[1] = std::nullopt;
      return true;
    }
    auto meso = PackageGameInstance::meso_;
    const auto& itm = items->at(index);
    auto itm2 = items->at(index);
    bool space = PackageGameInstance::LoadEmptyIndex(itm2.item).empty();
    if (meso < itm.price) {
      NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopbuyNoMeso;
    } else if (space) {
      NoticeUiSystem::OpenNoSpace(itm2.item->type);
      return true;
    } else {
      NoticeUiSystem::type_ = (itm.item->type == ItemEnum::kEquip)
                                  ? NoticeUiSystem::NoticeEnum::kShopbuy
                                  : NoticeUiSystem::NoticeEnum::kShopbuyMul;
      NoticeUiSystem::data_ = &itm;
    }
    NoticeUiSystem::Open();
    return true;
  }

  auto item1 = EventItemClick(event, {238, 129});
  if (item1.has_value()) {
    auto index = pages_[1] + item1.value();
    auto items = ShopGameInstance::LoadPkgItems(active_tab_[1]);
    if (index >= items.size()) {
      return true;
    }
    if (active_item_[1] != index) {
      active_item_[1] = index;
      active_item_[0] = std::nullopt;
      return true;
    }
    auto itm = items[index];
    if ((*itm)->type == ItemEnum::kEquip) {
      NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopbuySell;
    } else {
      auto num = ItemGameInstance::LoadItemNum(*itm);
      auto itm_info = ItemGameInstance::LoadItemInfo((*itm)->id, 0);
      bool unit_price = itm_info->get_child(u"unitPrice");
      if (num == 1 || unit_price) {
        NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopbuySell;
      } else {
        NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopbuySellMul;
      }
    }
    NoticeUiSystem::data_ = itm;
    NoticeUiSystem::Open();
  }
  return false;
}

bool ShopUiSystem::EventVscr(SDL_Event* event) {
  const uint32_t length = 235;
  auto x = (int)pos_.x + 212;
  auto y = (int)pos_.y + 129;

  int size = shop_->items.size() - 6;
  if (active_tab_[0] == 1) {
    size = ShopGameInstance::must_.size() - 6;
  }
  auto cursor_in = CursorGameInstance::cursor_ui_;
  bool top = cursor_in == Render;
  size = std::max(0, size);
  Scroll scroll(pages_[0], size, length);
  auto val = scroll.Click(x, y, top);
  pages_[0] = val;

  size = ShopGameInstance::LoadPkgItems(active_tab_[1]).size() - 6;
  x += 230;
  size = std::max(0, size);
  Scroll scroll1(pages_[1], size, length);
  val = scroll1.Click(x, y, top);
  pages_[1] = val;
  return true;
}

bool ShopUiSystem::EventTab(SDL_Event* event) {
  // 第一组标签页（商店分类）
  const static auto tab_node0 =
      WzResource::ui_->find(u"UIShop.img/Shop/TabBuy");
  std::vector<SDL_FRect> rects0;
  for (uint8_t i = 0; i < 2; i++) {
    rects0.push_back(SDL_FRect{static_cast<float>(int(pos_.x + 7 + i * 57)),
                               static_cast<float>(int(pos_.y + 96)), 57, 21});
  }
  Tab tab0{tab_node0, u"enabled", u"disabled", rects0,
           static_cast<uint8_t>(active_tab_[0])};
  if (auto i = tab0.Click()) {
    active_tab_[0] = *i;
    active_item_[0] = std::nullopt;
    pages_[0] = 0;
    return true;
  }

  // 第二组标签页（背包分类）
  const static auto tab_node1 =
      WzResource::ui_->find(u"UIShop.img/Shop/TabSell");
  std::vector<SDL_FRect> rects1;
  for (uint8_t i = 0; i < 4; i++) {
    rects1.push_back(SDL_FRect{static_cast<float>(int(pos_.x + 237 + i * 44)),
                               static_cast<float>(int(pos_.y + 96)), 44, 21});
  }
  Tab tab1{tab_node1, u"enabled", u"disabled", rects1,
           static_cast<uint8_t>(active_tab_[1])};
  if (auto i = tab1.Click()) {
    active_tab_[1] = *i;
    active_item_[1] = std::nullopt;
    pages_[1] = 0;
    return true;
  }

  return false;
}

void ShopUiSystem::EventVscrStart(SDL_Event* event) {
  SDL_FPoint lt{212, 129};
  const uint32_t length = 235;
  if (vscr_motion_[0] == false) {
    vscr_motion_[0] =
        Scroll(0, 0, length).ClickThumb(pos_.x + lt.x, pos_.y + lt.y);
  }
  lt.x += 230;
  if (vscr_motion_[1] == false) {
    vscr_motion_[1] =
        Scroll(0, 0, length).ClickThumb(pos_.x + lt.x, pos_.y + lt.y);
  }
}

void ShopUiSystem::EventVscrEnd() {
  vscr_motion_ = {};
}

void ShopUiSystem::EventVscrMove(SDL_Event* event) {
  auto mouse_state = Window::mouse_state_;
  SDL_FPoint lt{212, 129};
  const uint32_t length = 235;
  if (vscr_motion_[0]) {
    event->button.x = pos_.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos_.y + lt.y, pos_.y + lt.y + length);
    EventVscr(event);
  }
  if (vscr_motion_[1]) {
    lt.x += 230;
    event->button.x = pos_.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos_.y + lt.y, pos_.y + lt.y + length);
    EventVscr(event);
  }
}

bool ShopUiSystem::Event(SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = event->key.scancode;
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
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          if (EventItem(event)) {
            return false;
          }
          if (EventTab(event)) {
            return false;
          }
          EventButton(event);
          EventVscr(event);
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

}  // namespace sdlms
