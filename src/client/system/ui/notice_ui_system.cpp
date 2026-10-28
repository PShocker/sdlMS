#include "notice_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_shop.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/shop_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/shop_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "text_input_ui_system.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

void NoticeUiSystem::render_backgrnd() {
  wz::Node *node;
  switch (type) {
  case NoticeEnum::kWorldmapDisable:
  case NoticeEnum::kWorldmapTeleport:
  case NoticeEnum::kEquipNoAbility:
  case NoticeEnum::kEquipNoSpace:
  case NoticeEnum::kShopbuyNoMeso:
  case NoticeEnum::kNoEquipSpace:
  case NoticeEnum::kNoConsumeSpace:
  case NoticeEnum::kNoInstallSpace:
  case NoticeEnum::kNoEtcSpace:
  case NoticeEnum::kNoCashSpace:
  case NoticeEnum::kNoDecoSpace:
  case NoticeEnum::kNoStorageSpace:
  case NoticeEnum::kShopbuySell:
  case NoticeEnum::kShopbuySellNoNum:
  case NoticeEnum::kShopbuy:
  case NoticeEnum::kTradeBlock: {
    node = WzResource::ui->find(u"PopupWindow.img/Notice1");
    break;
  }
  case NoticeEnum::kApInc:
  case NoticeEnum::kShopbuySellMul:
  case NoticeEnum::kShopbuyMul:
  case NoticeEnum::kThrowMul: {
    node = WzResource::ui->find(u"PopupWindow.img/Notice2");
    break;
  }
  }
  auto [w, h] = load_wh();
  auto t = WzResource::load_texture(node->get_child(u"t"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  auto c = WzResource::load_texture(node->get_child(u"c"));
  auto s = WzResource::load_texture(node->get_child(u"s"));
  pos_rect = {
      pos.x,
      pos.y + t->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t->h - s->h),
  };
  SDL_RenderTextureTiled(Window::renderer, c, nullptr, 1, &pos_rect);

  pos_rect = {
      pos.x,
      pos.y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_RenderTexture(Window::renderer, s, nullptr, &pos_rect);
}

void NoticeUiSystem::render_button() {
  std::vector<wz::Node *> buttons_node = {};
  std::vector<SDL_FRect> buttons_rect = {};
  auto [w, h] = load_wh();
  switch (type) {
  case NoticeEnum::kWorldmapDisable:
  case NoticeEnum::kWorldmapTeleport:
  case NoticeEnum::kApInc:
  case NoticeEnum::kShopbuySell:
  case NoticeEnum::kShopbuySellMul:
  case NoticeEnum::kShopbuySellNoNum:
  case NoticeEnum::kShopbuy:
  case NoticeEnum::kShopbuyMul:
  case NoticeEnum::kShopbuyNoMeso:
  case NoticeEnum::kNoEquipSpace:
  case NoticeEnum::kNoConsumeSpace:
  case NoticeEnum::kNoInstallSpace:
  case NoticeEnum::kNoEtcSpace:
  case NoticeEnum::kNoCashSpace:
  case NoticeEnum::kNoDecoSpace:
  case NoticeEnum::kNoStorageSpace:
  case NoticeEnum::kEquipNoAbility:
  case NoticeEnum::kThrowMul:
  case NoticeEnum::kEquipNoSpace:
  case NoticeEnum::kTradeBlock: {
    buttons_node = {
        WzResource::ui->find(u"Basic.img/BtOK2"),
        WzResource::ui->find(u"Basic.img/BtCancel2"),
    };
    buttons_rect = {
        {w - 105, h - 30, 47, 18},
        {w - 55, h - 30, 47, 18},
    };
    break;
  }
  }
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render) {
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

void NoticeUiSystem::render_text() {
  std::u16string text = u"";
  SDL_FPoint p;
  switch (type) {
  case NoticeEnum::kShopbuy: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/buyItem");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kShopbuyMul: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/buyItemMul");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kShopbuyNoMeso: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/noMeso");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kNoEquipSpace: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/noEqpSpace");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kNoConsumeSpace: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/noConSpace");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kNoInstallSpace: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/noInsSpace");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kNoEtcSpace: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/noEtcSpace");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kNoCashSpace: {
    auto n =
        WzResource::ms->get_root()->find(u"String.img/Notice/noCashSpace");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kNoDecoSpace: {
    auto n =
        WzResource::ms->get_root()->find(u"String.img/Notice/noDecoSpace");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kNoStorageSpace: {
    auto n =
        WzResource::ms->get_root()->find(u"String.img/Notice/noStorageSpace");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kEquipNoAbility: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/noAbility");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kEquipNoSpace: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/noEqpSpace");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kShopbuySell: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/sellItem");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kShopbuySellMul: {
    auto n =
        WzResource::ms->get_root()->find(u"String.img/Notice/sellItemMul");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kShopbuySellNoNum: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/sellNoNum");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kApInc: {
    break;
  }
  case NoticeEnum::kWorldmapDisable: {
    auto n =
        WzResource::ms->get_root()->find(u"String.img/Notice/worldMapDis");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kWorldmapTeleport: {
    auto n =
        WzResource::ms->get_root()->find(u"String.img/Notice/worldMapTel");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kThrowMul: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/throwMul");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  case NoticeEnum::kTradeBlock: {
    auto n = WzResource::ms->get_root()->find(u"String.img/Notice/tradeBlock");
    text = static_cast<wz::Property<std::u16string> *>(n)->get();
    p = {20, 20};
    break;
  }
  default: {
    break;
  }
  }
  if (!text.empty()) {
    Freetype::load_aligned(true);
    Freetype::load_size(12);
    Freetype::load_color(255, 255, 255, 255);
    Freetype::draw_str(text, pos.x + p.x, pos.y + p.y, 240, 1.3);
  }
}

void NoticeUiSystem::render_input() {
  switch (type) {
  case NoticeEnum::kApInc:
  case NoticeEnum::kShopbuySellMul:
  case NoticeEnum::kShopbuyMul:
  case NoticeEnum::kThrowMul: {
    TextInputUiSystem::render(text, 5, 5);
    break;
  }
  default: {
    break;
  }
  }
}

bool NoticeUiSystem::render() {
  render_backgrnd();
  render_button();
  render_text();
  render_input();
  return true;
}

void NoticeUiSystem::open() {
  KeyboardInputSystem::reset();

  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    switch (type) {
    case NoticeEnum::kApInc:
    case NoticeEnum::kShopbuySell:
    case NoticeEnum::kShopbuySellMul:
    case NoticeEnum::kShopbuyMul:
    case NoticeEnum::kThrowMul: {
      text = {
          .max_size = 12,
          .text = u"1",
          .composition = {},
          .disable = false,
          .active = false,
          .r =
              SDL_Rect{
                  static_cast<int>(pos.x + 18),
                  static_cast<int>(pos.y + 40),
                  230,
                  30,
              },
          .font_color = {0, 0, 0, 255},
          .cur_color = {0, 0, 0, 255},
          .font_size = 12,
      };
      text.type.reset();
      text.type.set(TextInput::kDigit);
      text.cur = 1;
      TextInputUiSystem::active(text);
      break;
    }
    default: {
      break;
    }
    }

    switch (type) {
    case NoticeEnum::kThrowMul:
    case NoticeEnum::kShopbuySellMul: {
      auto p =
          std::any_cast<std::polymorphic<GameItem> *>(NoticeUiSystem::data);
      auto num = ItemGameInstance::load_item_num(*p);
      auto tmp = std::to_string(num);
      text.text = {tmp.begin(), tmp.end()};
      break;
    }
    default: {
      break;
    }
    }
    AudioGameInstance::load_audio(u"UI.img/DlgNotice", 0);
    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void NoticeUiSystem::open_no_space(ItemEnum type) {
  switch (type) {
  case ItemEnum::kEquip: {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kNoEquipSpace;
    break;
  }
  case ItemEnum::kConsume: {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kNoConsumeSpace;
    break;
  }
  case ItemEnum::kInstall: {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kNoInstallSpace;
    break;
  }
  case ItemEnum::kEtc: {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kNoEtcSpace;
    break;
  }
  case ItemEnum::kCash: {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kNoCashSpace;
    break;
  }
  case ItemEnum::kDeco: {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kNoDecoSpace;
    break;
  }
  }
  if (!std::ranges::contains(System::render_systems, &render)) {
    open();
  }
}

void NoticeUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
}

SDL_FPoint NoticeUiSystem::load_wh() {
  switch (type) {
  case NoticeEnum::kWorldmapDisable:
  case NoticeEnum::kWorldmapTeleport:
  case NoticeEnum::kShopbuySell:
  case NoticeEnum::kShopbuySellNoNum:
  case NoticeEnum::kShopbuy:
  case NoticeEnum::kShopbuyNoMeso:
  case NoticeEnum::kNoEquipSpace:
  case NoticeEnum::kNoConsumeSpace:
  case NoticeEnum::kNoInstallSpace:
  case NoticeEnum::kNoEtcSpace:
  case NoticeEnum::kNoCashSpace:
  case NoticeEnum::kNoDecoSpace:
  case NoticeEnum::kNoStorageSpace:
  case NoticeEnum::kEquipNoAbility:
  case NoticeEnum::kEquipNoSpace:
  case NoticeEnum::kTradeBlock: {
    return {266, 116};
  }
  case NoticeEnum::kApInc:
  case NoticeEnum::kShopbuySellMul:
  case NoticeEnum::kShopbuyMul:
  case NoticeEnum::kThrowMul:
    return {266, 119};
  }
  return {0, 0};
}

void NoticeUiSystem::event_close() { close(); }

void NoticeUiSystem::event_button_shopbuy() {
  auto p = std::any_cast<const GameShopItem *>(NoticeUiSystem::data);
  auto itm = p->item;
  switch (type) {
  case NoticeEnum::kShopbuy: {
    PackageGameInstance::add_item(itm);
    PackageGameInstance::meso -= p->price;
    break;
  }
  case NoticeEnum::kShopbuyMul: {
    auto num = std::stoi(std::string{text.text.begin(), text.text.end()});
    ItemGameInstance::set_item_num(itm, num);
    auto price = p->price * num;
    if (PackageGameInstance::meso < price) {
      type = NoticeEnum::kShopbuyNoMeso;
      return;
      break;
    }
    auto buy_num = std::stoi(std::string{text.text.begin(), text.text.end()});
    if (ShopUiSystem::active_tab[0] == 1) {
      auto &must = ShopGameInstance::must;
      auto &must_itm = must[ShopUiSystem::active_item[0].value()];
      auto itm_num = ItemGameInstance::load_item_num(must_itm.item);
      if (buy_num > itm_num) {
        type = NoticeEnum::kShopbuySellNoNum;
        return;
      }
    }
    if (PackageGameInstance::add_item(itm)) {
      PackageGameInstance::meso -= price;
      if (ShopUiSystem::active_tab[0] == 1) {
        auto &must = ShopGameInstance::must;
        auto &must_itm = must[ShopUiSystem::active_item[0].value()];
        ItemGameInstance::dec_item_num(must_itm.item, buy_num);
      }
    } else {
      open_no_space(itm->type);
      return;
    }
    break;
  }
  default: {
    break;
  }
  }
  if (ShopUiSystem::active_tab[0] == 1) {
    auto &must = ShopGameInstance::must;
    auto &itm = must[ShopUiSystem::active_item[0].value()].item;
    if (ItemGameInstance::load_item_num(itm) == 0) {
      must.erase(must.begin() + ShopUiSystem::active_item[0].value());
    }
    ShopUiSystem::pages[0] = 0;
  }
  ShopUiSystem::active_tab[1] = (int)itm->type;
  ShopUiSystem::pages[1] = 0;
  ShopUiSystem::active_item = {};
  close();
}

void NoticeUiSystem::event_button_shopbuy_sell() {
  auto p = std::any_cast<std::polymorphic<GameItem> *>(NoticeUiSystem::data);
  GameShopItem gst = ShopGameInstance::load_shop_item((*p)->id);
  gst.item = *p;
  switch (type) {
  case NoticeEnum::kShopbuySell: {
    auto itm_info = ItemGameInstance::load_item_info(gst.item->id, 0);
    auto unitPrice = itm_info->get_child(u"unitPrice");
    auto num = ItemGameInstance::load_item_num(gst.item);
    if (unitPrice) {
      auto u_price = static_cast<wz::Property<double> *>(unitPrice)->get();
      PackageGameInstance::add_meso(u_price * num);
    } else {
      PackageGameInstance::add_meso(gst.price);
    }
    if ((*p)->id.starts_with(u"0207")) {
      (*p)->id = u"";
    } else {
      ItemGameInstance::dec_item_num(*p, num);
    }
    break;
  }
  case NoticeEnum::kShopbuySellMul: {
    auto num = ItemGameInstance::load_item_num(gst.item);
    auto sell_num = std::stoi(std::string{text.text.begin(), text.text.end()});
    if (sell_num > num) {
      type = NoticeEnum::kShopbuySellNoNum;
      return;
    }
    PackageGameInstance::add_meso(gst.price * num);
    ItemGameInstance::dec_item_num(*p, sell_num);
    ItemGameInstance::add_item_num(gst.item, (sell_num - num));
    break;
  }
  default: {
    break;
  }
  }
  ShopGameInstance::add_must_item(gst.item);
  ShopUiSystem::active_tab[0] = 1;
  ShopUiSystem::active_item = {};
  ShopUiSystem::pages = {};
  close();
}

void NoticeUiSystem::event_button_ap_inc() {
  auto num = std::stoi(std::string{text.text.begin(), text.text.end()});
  auto r_ap = CharacterStatGameInstance::get_remain_ap();
  if (num > r_ap) {
    type = NoticeEnum::kShopbuySellNoNum;
    return;
  }
  auto p = std::any_cast<int>(NoticeUiSystem::data);
  switch (p) {
  case 0: {
    // hp
    auto sf_hp = CharacterStatGameInstance::get_hp_ap();
    CharacterStatGameInstance::set_hp_ap(sf_hp + num);
    break;
  };
  case 1: {
    // mp
    auto sf_mp = CharacterStatGameInstance::get_mp_ap();
    CharacterStatGameInstance::set_mp_ap(sf_mp + num);
    break;
  };
  case 2: {
    // str
    auto sf_str = CharacterStatGameInstance::get_str_ap();
    CharacterStatGameInstance::set_str_ap(sf_str + num);
    break;
  };
  case 3: {
    // dex
    auto sf_dex = CharacterStatGameInstance::get_dex_ap();
    CharacterStatGameInstance::set_dex_ap(sf_dex + num);
    break;
  };
  case 4: {
    // int
    auto sf_int = CharacterStatGameInstance::get_int_ap();
    CharacterStatGameInstance::set_int_ap(sf_int + num);
    break;
  };
  case 5: {
    // luk
    auto sf_luk = CharacterStatGameInstance::get_luk_ap();
    CharacterStatGameInstance::set_luk_ap(sf_luk + num);
    break;
  };
  }
  CharacterStatGameInstance::set_remain_ap(r_ap - num);
  CharacterStatGameInstance::update();
  close();
}

void NoticeUiSystem::event_button_worldmap_teleport() {
  auto p = std::any_cast<uint32_t>(NoticeUiSystem::data);
  SceneSystemInstance::enter_prepare(p, u"sp", 0);
}

void NoticeUiSystem::event_button_throw_mul() {
  int itm_num;
  std::u16string itm_id;
  if (std::any_cast<std::nullptr_t>(&NoticeUiSystem::data)) {
    itm_id = u"0";
    itm_num = PackageGameInstance::meso;
  } else {
    auto p =
        std::any_cast<std::polymorphic<GameItem> *>(NoticeUiSystem::data);
    itm_id = (*p)->id;
    itm_num = ItemGameInstance::load_item_num(*p);
  }
  auto throw_num = std::stoi(std::string{text.text.begin(), text.text.end()});
  if (throw_num > itm_num) {
    type = NoticeEnum::kShopbuySellNoNum;
    CursorGameInstance::cursor_hand = std::nullopt;
    return;
  } else {
    DropT dt;
    ItemT it;
    it.item_id = std::stoi(std::string{itm_id.begin(), itm_id.end()});
    it.item_num = throw_num;
    dt.drop.Set(it);

    dt.x1 = CharacterGameInstance::self.pos.x;
    dt.y1 = CharacterGameInstance::self.pos.y;

    dt.page = CharacterGameInstance::self.page;

    ClientCharacterDropT cct;
    cct.map_id = SceneSystemInstance::map_id;
    cct.payload = std::make_unique<DropT>(dt);
    ClientRequest::send_to_host(cct);

    CursorGameInstance::cursor_hand_net = {
        .type = CursorGameInstance::CursorHandNetType::kDrop,
    };
    close();
  }
}

void NoticeUiSystem::event_button_throw_close() {
  CursorGameInstance::cursor_hand = std::nullopt;
  close();
}

void NoticeUiSystem::event_button_equip_no_ap() {
  CursorGameInstance::cursor_hand = std::nullopt;
  close();
}

bool NoticeUiSystem::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> buttons_rect;
  std::vector<std::function<void()>> func = {};
  auto [w, h] = load_wh();
  buttons_rect = {
      {w - 105, h - 30, 47, 18},
      {w - 55, h - 30, 47, 18},
  };
  switch (type) {
  case NoticeEnum::kShopbuy:
  case NoticeEnum::kShopbuyMul: {
    func = {event_button_shopbuy, close};
    break;
  }
  case NoticeEnum::kApInc: {
    func = {event_button_shopbuy_sell, close};
    break;
  }
  case NoticeEnum::kWorldmapTeleport: {
    func = {event_button_worldmap_teleport, close};
    break;
  }
  case NoticeEnum::kShopbuySell:
  case NoticeEnum::kShopbuySellMul: {
    func = {event_button_shopbuy_sell, close};
    break;
  }
  case NoticeEnum::kThrowMul: {
    func = {event_button_throw_mul, event_button_throw_close};
    break;
  }
  case NoticeEnum::kEquipNoAbility: {
    func = {event_button_equip_no_ap, event_button_equip_no_ap};
    break;
  }
  default: {
    func = {close, close};
    break;
  }
  }
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      func[i]();
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }
  return false;
}

void NoticeUiSystem::event_input(SDL_Event *event) {
  switch (type) {
  case NoticeEnum::kApInc:
  case NoticeEnum::kShopbuySellMul:
  case NoticeEnum::kShopbuyMul:
  case NoticeEnum::kThrowMul: {
    TextInputUiSystem::event(event, text);
  }
  default: {
    break;
  }
  }
}

void NoticeUiSystem::event_motion_bt_audio(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  std::vector<SDL_FRect> buttons_rect;
  auto [w, h] = load_wh();
  buttons_rect = {
      {w - 105, h - 30, 47, 18},
      {w - 55, h - 30, 47, 18},
  };
  static int8_t bt_audio = -1;
  auto &mouse_pos = Window::mouse_pos;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      bt_in = i;
      break;
    }
  }
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::load_audio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool NoticeUiSystem::event(SDL_Event *event) {
  event_input(event);
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      event_close();
      if (type == NoticeEnum::kThrowMul) {
        CursorGameInstance::cursor_hand = std::nullopt;
      }
      break;
    }
    case SDL_SCANCODE_RETURN: {
      switch (type) {
      case NoticeEnum::kShopbuy:
      case NoticeEnum::kShopbuyMul: {
        event_button_shopbuy();
        break;
      }
      case NoticeEnum::kShopbuySell:
      case NoticeEnum::kShopbuySellMul: {
        event_button_shopbuy_sell();
        break;
      }
      case NoticeEnum::kApInc: {
        break;
      }
      case NoticeEnum::kWorldmapTeleport: {
        event_button_worldmap_teleport();
        break;
      }
      case NoticeEnum::kThrowMul: {
        event_button_throw_mul();
        break;
      }
      default: {
        close();
        break;
      }
      }
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
        event_button(event);
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_motion_bt_audio(event);
    break;
  }
  default: {
    break;
  }
  }

  return false;
}

bool NoticeUiSystem::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}