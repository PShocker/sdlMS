#include "src/client/system/ui/notice_ui_system.h"

#include "src/client/game/game_shop.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/shop_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/text_input.h"
#include "src/client/system/ui/component/ui_component.h"
#include "src/client/system/ui/shop_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>


namespace sdlms {

void NoticeUiSystem::RenderBackgrnd() {
  wz::Node* node;
  switch (type_) {
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
    case NoticeEnum::kQuestGiveup:
    case NoticeEnum::kShopRecharge: {
      node = WzResource::ui_->find(u"PopupWindow.img/Notice1");
      break;
    }
    case NoticeEnum::kApInc:
    case NoticeEnum::kShopbuySellMul:
    case NoticeEnum::kShopbuyMul:
    case NoticeEnum::kThrowMul: {
      node = WzResource::ui_->find(u"PopupWindow.img/Notice2");
      break;
    }
  }
  auto [w, h] = LoadWh();
  auto t = WzResource::LoadTexture(node->get_child(u"t"));
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
  auto c = WzResource::LoadTexture(node->get_child(u"c"));
  auto s = WzResource::LoadTexture(node->get_child(u"s"));
  pos_rect = {
      pos_.x,
      pos_.y + t->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t->h - s->h),
  };
  SDL_RenderTextureTiled(Window::renderer_, c, nullptr, 1, &pos_rect);

  pos_rect = {
      pos_.x,
      pos_.y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_RenderTexture(Window::renderer_, s, nullptr, &pos_rect);
}


UIComponents& NoticeUiSystem::GetComponent() {
  components_.clear();
  std::vector<wz::Node*> buttons_node = {};
  std::vector<SDL_FRect> buttons_rect = {};
  auto [w, h] = LoadWh();
  switch (type_) {
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
    case NoticeEnum::kQuestGiveup:
    case NoticeEnum::kShopRecharge: {
      buttons_node = {
          WzResource::ui_->find(u"Basic.img/BtOK2"),
          WzResource::ui_->find(u"Basic.img/BtCancel2"),
      };
      buttons_rect = {
          {w - 105, h - 30, 47, 18},
          {w - 55, h - 30, 47, 18},
      };
      break;
    }
  }
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x += (int)pos_.x;
    rect.y += (int)pos_.y;
    Button button{.node = buttons_node[i],
                  .rect = rect,
                  .enable = true,
                  .focused = CursorGameInstance::cursor_ui_ == Render};
    components_.push_back(button);
  }

  switch (type_) {
    case NoticeEnum::kShopbuy:
    case NoticeEnum::kShopbuyMul: {
      std::get<Button>(components_[0]).callback = EventButtonShopbuy;
      std::get<Button>(components_[1]).callback = Close;
      break;
    }
    case NoticeEnum::kApInc: {
      std::get<Button>(components_[0]).callback = EventButtonApInc;
      std::get<Button>(components_[1]).callback = Close;
      break;
    }
    case NoticeEnum::kWorldmapTeleport: {
      std::get<Button>(components_[0]).callback = EventButtonWorldmapTeleport;
      std::get<Button>(components_[1]).callback = Close;
      break;
    }
    case NoticeEnum::kShopbuySell:
    case NoticeEnum::kShopbuySellMul: {
      std::get<Button>(components_[0]).callback = EventButtonShopbuySell;
      std::get<Button>(components_[1]).callback = Close;
      break;
    }
    case NoticeEnum::kThrowMul: {
      std::get<Button>(components_[0]).callback = EventButtonThrowMul;
      std::get<Button>(components_[1]).callback = EventButtonThrowClose;
      break;
    }
    case NoticeEnum::kQuestGiveup: {
      std::get<Button>(components_[0]).callback = EventButtonQuestGiveup;
      std::get<Button>(components_[1]).callback = Close;
      break;
    }
    case NoticeEnum::kShopRecharge: {
      std::get<Button>(components_[0]).callback = EventButtonShopRecharge;
      std::get<Button>(components_[1]).callback = Close;
      break;
    }
    case NoticeEnum::kEquipNoAbility: {
      std::get<Button>(components_[0]).callback = EventButtonEquipNoAp;
      std::get<Button>(components_[1]).callback = EventButtonEquipNoAp;
      break;
    }
    default: {
      std::get<Button>(components_[0]).callback = Close;
      std::get<Button>(components_[1]).callback = Close;
      break;
    }
  }

  return components_;
}


void NoticeUiSystem::RenderText() {
  std::u16string text = u"";
  SDL_FPoint p;
  switch (type_) {
    case NoticeEnum::kShopbuy: {
      auto n = WzResource::ms_->get_root()->find(u"String.img/Notice/buyItem");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kShopbuyMul: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/buyItemMul");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kShopbuyNoMeso: {
      auto n = WzResource::ms_->get_root()->find(u"String.img/Notice/noMeso");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kNoEquipSpace: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/noEqpSpace");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kNoConsumeSpace: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/noConSpace");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kNoInstallSpace: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/noInsSpace");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kNoEtcSpace: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/noEtcSpace");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kNoCashSpace: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/noCashSpace");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kNoDecoSpace: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/noDecoSpace");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kNoStorageSpace: {
      auto n = WzResource::ms_->get_root()->find(
          u"String.img/Notice/noStorageSpace");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kEquipNoAbility: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/noAbility");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kEquipNoSpace: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/noEqpSpace");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kShopbuySell: {
      auto n = WzResource::ms_->get_root()->find(u"String.img/Notice/sellItem");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kShopbuySellMul: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/sellItemMul");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kShopbuySellNoNum: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/sellNoNum");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kApInc: {
      // 角色加点提示框固定文案
      text = u"Please enter the number of AP to add.";
      p = {20, 20};
      break;
    }
    case NoticeEnum::kWorldmapDisable: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/worldMapDis");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kWorldmapTeleport: {
      auto n =
          WzResource::ms_->get_root()->find(u"String.img/Notice/worldMapTel");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kThrowMul: {
      auto n = WzResource::ms_->get_root()->find(u"String.img/Notice/throwMul");
      text = static_cast<wz::Property<std::u16string>*>(n)->get();
      p = {20, 20};
      break;
    }
    case NoticeEnum::kQuestGiveup: {
      text = u"Do you want to forfeit this quest?";
      p = {20, 20};
      break;
    }
    case NoticeEnum::kShopRecharge: {
      uint64_t cost = 0;
      if (auto p = std::any_cast<uint64_t>(&data_); p != nullptr) {
        cost = *p;
      }
      auto num = TextGameInstance::FormatWithCommas(cost);
      text = u"Recharge this throwing star to full?\nCost: " +
             TextGameInstance::LoadU16str(num.c_str()) + u" mesos";
      p = {20, 20};
      break;
    }
    default: {
      break;
    }
  }
  if (!text.empty()) {
    Freetype::LoadAligned(true);
    Freetype::LoadSize(12);
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawStr(text, pos_.x + p.x, pos_.y + p.y, 240, 1.3);
  }
}

void NoticeUiSystem::RenderInput() {
  switch (type_) {
    case NoticeEnum::kApInc:
    case NoticeEnum::kShopbuySellMul:
    case NoticeEnum::kShopbuyMul:
    case NoticeEnum::kThrowMul: {
      text_.Render();
      break;
    }
    default: {
      break;
    }
  }
}

bool NoticeUiSystem::Render() {
  RenderBackgrnd();
  RenderUI();
  RenderText();
  RenderInput();
  return true;
}

void NoticeUiSystem::Open() {
  KeyboardInputSystem::Reset();

  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    switch (type_) {
      case NoticeEnum::kApInc:
      case NoticeEnum::kShopbuySell:
      case NoticeEnum::kShopbuySellMul:
      case NoticeEnum::kShopbuyMul:
      case NoticeEnum::kThrowMul: {
        text_ = {
            .max_size = 9,
            .text = u"1",
            .composition = {},
            .disable = false,
            .active = false,
            .r =
                SDL_Rect{
                    static_cast<int>(pos_.x + 18),
                    static_cast<int>(pos_.y + 40),
                    230,
                    30,
                },
            .font_color = {0, 0, 0, 255},
            .cur_color = {0, 0, 0, 255},
            .font_size = 12,
        };
        text_.type.reset();
        text_.type.set(TextInput::kDigit);
        text_.cur = 1;
        text_.Active();
        break;
      }
      default: {
        break;
      }
    }

    switch (type_) {
      case NoticeEnum::kThrowMul:
      case NoticeEnum::kShopbuySellMul: {
        auto p =
            std::any_cast<std::polymorphic<GameItem>*>(NoticeUiSystem::data_);
        auto num = ItemGameInstance::LoadItemNum(*p);
        auto tmp = std::to_string(num);
        text_.text = {tmp.begin(), tmp.end()};
        text_.cur = text_.text.size();
        break;
      }
      default: {
        break;
      }
    }
    AudioGameInstance::LoadAudio(u"UI.img/DlgNotice", 0);
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void NoticeUiSystem::OpenNoSpace(ItemEnum type) {
  switch (type) {
    case ItemEnum::kEquip: {
      NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kNoEquipSpace;
      break;
    }
    case ItemEnum::kConsume: {
      NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kNoConsumeSpace;
      break;
    }
    case ItemEnum::kInstall: {
      NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kNoInstallSpace;
      break;
    }
    case ItemEnum::kEtc: {
      NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kNoEtcSpace;
      break;
    }
    case ItemEnum::kCash: {
      NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kNoCashSpace;
      break;
    }
    case ItemEnum::kDeco: {
      NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kNoDecoSpace;
      break;
    }
  }
  if (!std::ranges::contains(System::render_systems_, &Render)) {
    Open();
  }
}

void NoticeUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  on_confirm_ = nullptr;
}

SDL_FPoint NoticeUiSystem::LoadWh() {
  switch (type_) {
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
    case NoticeEnum::kQuestGiveup:
    case NoticeEnum::kShopRecharge: {
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

void NoticeUiSystem::EventClose() {
  Close();
}

void NoticeUiSystem::EventButtonShopbuy() {
  auto p = std::any_cast<const GameShopItem*>(NoticeUiSystem::data_);
  auto itm = p->item;
  std::optional<int> must_index;
  if (ShopUiSystem::active_tab_[0] == 1 &&
      ShopUiSystem::active_item_[0].has_value()) {
    must_index = ShopUiSystem::active_item_[0].value();
  }
  switch (type_) {
    case NoticeEnum::kShopbuy: {
      if (!PackageGameInstance::AddItem(itm)) {
        OpenNoSpace(itm->type);
        return;
      }
      PackageGameInstance::meso_ -= p->price;
      break;
    }
    case NoticeEnum::kShopbuyMul: {
      auto num = std::stoi(std::string{text_.text.begin(), text_.text.end()});
      ItemGameInstance::SetItemNum(itm, num);
      auto price = static_cast<uint64_t>(p->price) * num;
      if (PackageGameInstance::meso_ < price) {
        type_ = NoticeEnum::kShopbuyNoMeso;
        return;
        break;
      }
      if (must_index.has_value()) {
        auto& must = ShopGameInstance::must_;
        if (*must_index < 0 || *must_index >= static_cast<int>(must.size())) {
          return;
        }
        auto itm_num =
            ItemGameInstance::LoadItemNum(must[*must_index].item);
        if (num > itm_num) {
          type_ = NoticeEnum::kShopbuySellNoNum;
          return;
        }
      }
      if (PackageGameInstance::AddItem(itm)) {
        PackageGameInstance::meso_ -= price;
      } else {
        OpenNoSpace(itm->type);
        return;
      }
      break;
    }
    default: {
      break;
    }
  }
  // 从 Must 买回后删除对应记录，而不是只把数量减为 0。
  if (must_index.has_value()) {
    auto& must = ShopGameInstance::must_;
    if (*must_index >= 0 && *must_index < static_cast<int>(must.size())) {
      must.erase(must.begin() + *must_index);
    }
    ShopUiSystem::pages_[0] = 0;
  }
  ShopUiSystem::active_tab_[1] = (int)itm->type;
  ShopUiSystem::pages_[1] = 0;
  ShopUiSystem::active_item_ = {};
  Close();
}

void NoticeUiSystem::EventButtonShopbuySell() {
  auto p = std::any_cast<std::polymorphic<GameItem>*>(NoticeUiSystem::data_);
  GameShopItem gst = ShopGameInstance::LoadShopItem((*p)->id);
  gst.item = *p;
  switch (type_) {
    case NoticeEnum::kShopbuySell: {
      if (gst.item->type == ItemEnum::kEquip) {
        // 装备没有 unitPrice，也不能走 ItemGameInstance::LoadItemInfo。
        PackageGameInstance::AddMeso(gst.price);
        ItemGameInstance::DecItemNum(*p, 1);
      } else {
        auto itm_info = ItemGameInstance::LoadItemInfo(gst.item->id, 0);
        auto unit_price = itm_info->get_child(u"unitPrice");
        auto num = ItemGameInstance::LoadItemNum(gst.item);
        if (unit_price) {
          auto u_price = static_cast<wz::Property<double>*>(unit_price)->get();
          PackageGameInstance::AddMeso(u_price * num);
        } else {
          PackageGameInstance::AddMeso(gst.price);
        }
        if ((*p)->id.starts_with(u"0207")) {
          (*p)->id = u"";
          // 0207 的 DecItemNum 会保留 id，出售时必须把数量一并清零，
          // 否则买回时 AddItemSlot 会把旧数量和新数量相加（3 变 6）。
          ItemGameInstance::SetItemNum(*p, 0);
        } else {
          ItemGameInstance::DecItemNum(*p, num);
        }
      }
      break;
    }
    case NoticeEnum::kShopbuySellMul: {
      auto num = ItemGameInstance::LoadItemNum(gst.item);
      auto sell_num =
          std::stoi(std::string{text_.text.begin(), text_.text.end()});
      if (sell_num > num) {
        type_ = NoticeEnum::kShopbuySellNoNum;
        return;
      }
      PackageGameInstance::AddMeso(gst.price * num);
      ItemGameInstance::DecItemNum(*p, sell_num);
      ItemGameInstance::AddItemNum(gst.item, (sell_num - num));
      break;
    }
    default: {
      break;
    }
  }
  ShopGameInstance::AddMustItem(gst.item);
  ShopUiSystem::active_tab_[0] = 1;
  ShopUiSystem::active_item_ = {};
  ShopUiSystem::pages_ = {};
  Close();
}

void NoticeUiSystem::EventButtonApInc() {
  if (text_.text.empty()) {
    return;
  }
  auto num = std::stoi(std::string{text_.text.begin(), text_.text.end()});
  if (num <= 0) {
    return;
  }
  auto r_ap = CharacterStatGameInstance::GetRemainAp();
  if (num > r_ap) {
    type_ = NoticeEnum::kShopbuySellNoNum;
    return;
  }
  auto p = std::any_cast<int>(NoticeUiSystem::data_);
  switch (p) {
    case 0: {
      // hp
      auto sf_hp = CharacterStatGameInstance::GetHpAp();
      CharacterStatGameInstance::SetHpAp(sf_hp + num);
      break;
    };
    case 1: {
      // mp
      auto sf_mp = CharacterStatGameInstance::GetMpAp();
      CharacterStatGameInstance::SetMpAp(sf_mp + num);
      break;
    };
    case 2: {
      // str
      auto sf_str = CharacterStatGameInstance::GetStrAp();
      CharacterStatGameInstance::SetStrAp(sf_str + num);
      break;
    };
    case 3: {
      // dex
      auto sf_dex = CharacterStatGameInstance::GetDexAp();
      CharacterStatGameInstance::SetDexAp(sf_dex + num);
      break;
    };
    case 4: {
      // int
      auto sf_int = CharacterStatGameInstance::GetIntAp();
      CharacterStatGameInstance::SetIntAp(sf_int + num);
      break;
    };
    case 5: {
      // luk
      auto sf_luk = CharacterStatGameInstance::GetLukAp();
      CharacterStatGameInstance::SetLukAp(sf_luk + num);
      break;
    };
  }
  CharacterStatGameInstance::SetRemainAp(r_ap - num);
  CharacterStatGameInstance::Update();
  Close();
}

void NoticeUiSystem::EventButtonWorldmapTeleport() {
  auto p = std::any_cast<uint32_t>(NoticeUiSystem::data_);
  SceneSystemInstance::EnterPrepare(p, u"sp", 0);
}

void NoticeUiSystem::EventButtonThrowMul() {
  uint64_t itm_num = 0;
  std::u16string itm_id;
  if (std::any_cast<std::nullptr_t>(&NoticeUiSystem::data_)) {
    itm_id = u"0";
    itm_num = PackageGameInstance::meso_;
  } else {
    auto p = std::any_cast<std::polymorphic<GameItem>*>(NoticeUiSystem::data_);
    itm_id = (*p)->id;
    itm_num = static_cast<uint64_t>(ItemGameInstance::LoadItemNum(*p));
  }
  auto throw_num = std::stoi(std::string{text_.text.begin(), text_.text.end()});
  if (throw_num <= 0) {
    return;
  }
  if (static_cast<uint64_t>(throw_num) > itm_num) {
    type_ = NoticeEnum::kShopbuySellNoNum;
    CursorGameInstance::cursor_hand_ = std::nullopt;
    return;
  } else {
    fbs::DropT dt;
    fbs::ItemT it;
    it.item_id = std::stoi(std::string{itm_id.begin(), itm_id.end()});
    it.item_num = throw_num;
    dt.drop.Set(it);

    dt.x1 = CharacterGameInstance::self_.pos.x;
    dt.y1 = CharacterGameInstance::self_.pos.y;

    dt.page = CharacterGameInstance::self_.page;

    fbs::ClientCharacterDropT cct;
    cct.map_id = SceneSystemInstance::map_id_;
    cct.payload = std::make_unique<fbs::DropT>(dt);
    ClientRequest::SendToHost(cct);

    CursorGameInstance::cursor_hand_net_ = {
        .type = CursorGameInstance::CursorHandNetType::kDrop,
    };
    Close();
  }
}

void NoticeUiSystem::EventButtonThrowClose() {
  CursorGameInstance::cursor_hand_ = std::nullopt;
  Close();
}

void NoticeUiSystem::EventButtonQuestGiveup() {
  auto confirm = std::move(on_confirm_);
  on_confirm_ = nullptr;
  Close();
  if (confirm) {
    confirm();
  }
}

void NoticeUiSystem::EventButtonShopRecharge() {
  auto confirm = std::move(on_confirm_);
  on_confirm_ = nullptr;
  Close();
  if (confirm) {
    confirm();
  }
}

void NoticeUiSystem::EventButtonEquipNoAp() {
  CursorGameInstance::cursor_hand_ = std::nullopt;
  Close();
}

bool NoticeUiSystem::EventUI(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), true);
}


void NoticeUiSystem::EventInput(SDL_Event* event) {
  switch (type_) {
    case NoticeEnum::kApInc:
    case NoticeEnum::kShopbuySellMul:
    case NoticeEnum::kShopbuyMul:
    case NoticeEnum::kThrowMul: {
      text_.Event(event);
    }
    default: {
      break;
    }
  }
}

void NoticeUiSystem::EventMotionBtAudio(SDL_Event* event) {
  static int8_t bt_audio = -1;
  int8_t bt_in = static_cast<int8_t>(ButtonHoverIndex(GetComponent()));
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool NoticeUiSystem::Event(SDL_Event* event) {
  EventInput(event);
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = event->key.scancode;
      switch (scan_code) {
        case SDL_SCANCODE_ESCAPE: {
          EventClose();
          if (type_ == NoticeEnum::kThrowMul) {
            CursorGameInstance::cursor_hand_ = std::nullopt;
          }
          break;
        }
        case SDL_SCANCODE_RETURN: {
          switch (type_) {
            case NoticeEnum::kShopbuy:
            case NoticeEnum::kShopbuyMul: {
              EventButtonShopbuy();
              break;
            }
            case NoticeEnum::kShopbuySell:
            case NoticeEnum::kShopbuySellMul: {
              EventButtonShopbuySell();
              break;
            }
            case NoticeEnum::kApInc: {
              EventButtonApInc();
              break;
            }
            case NoticeEnum::kWorldmapTeleport: {
              EventButtonWorldmapTeleport();
              break;
            }
            case NoticeEnum::kThrowMul: {
              EventButtonThrowMul();
              break;
            }
            case NoticeEnum::kQuestGiveup: {
              EventButtonQuestGiveup();
              break;
            }
            case NoticeEnum::kShopRecharge: {
              EventButtonShopRecharge();
              break;
            }
            default: {
              Close();
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
        if (CursorGameInstance::cursor_ui_ == Render) {
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          EventUI(event);
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventMotionBtAudio(event);
      break;
    }
    default: {
      break;
    }
  }

  return false;
}

bool NoticeUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

}  // namespace sdlms
