#include "shop_game_instance.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_shop.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <flat_map>
#include <optional>
#include <string>
namespace sdlms {


std::polymorphic<GameItem>
ShopGameInstance::LoadItem(const std::u16string &id) {
  std::polymorphic<GameItem> r;
  if (ItemGameInstance::CheckItem(id)) {
    r = ItemGameInstance::LoadItem(id, 0);
  } else {
    r = EquipGameInstance::LoadItem(id);
  }
  return r;
}

GameShopItem ShopGameInstance::LoadShopItem(const std::u16string &id) {
  auto item = ShopGameInstance::LoadItem(id);
  GameShopItem gst;
  if (item->type == ItemEnum::kEquip) {
    auto info = EquipGameInstance::LoadEquipInfo(item->id);
    gst.price =
        static_cast<wz::Property<int> *>(info->get_child(u"price"))->get();
  } else {
    auto info = ItemGameInstance::LoadItemInfo(item->id, 0);
    if (info->get_child(u"price")) {
      gst.price =
          static_cast<wz::Property<int> *>(info->get_child(u"price"))->get();
    } else {
      gst.price = 1;
    }
  }
  gst.item = item;
  return gst;
}

GameShop ShopGameInstance::LoadShop(const std::u16string &shop_id) {
  GameShop shop;
  auto node =
      WzResource::ms_->get_root()->find(u"Shop.img/ShopItem/" + shop_id);
  for (auto [k, v] : *node->get_children()) {
    auto node = v[0];
    auto price =
        static_cast<wz::Property<int> *>(node->get_child(u"price"))->get();
    auto id =
        static_cast<wz::Property<std::u16string> *>(node->get_child(u"itemid"))
            ->get();
    GameShopItem item;
    item.price = price;
    item.item = LoadItem(id);
    shop.items.push_back(std::move(item));
  }
  return shop;
}

const GameShop *
ShopGameInstance::LoadNpcShop(const std::u16string &npc_id) {
  std::optional<GameShop> shop;
  static std::flat_map<std::u16string, GameShop> cache;
  if (cache.empty()) {
    auto node = WzResource::ms_->get_root()->find(u"Shop.img/ShopNpc");
    for (auto [k, v] : *node->get_children()) {
      auto npc_i = static_cast<wz::Property<std::u16string> *>(v[0])->get();
      cache.emplace(std::move(npc_i), LoadShop(k));
    }
  }
  if (cache.contains(npc_id)) {
    return &cache.at(npc_id);
  } else {
    return nullptr;
  }
}

bool ShopGameInstance::AddMustItem(std::polymorphic<GameItem> &item) {
  auto add_num = ItemGameInstance::LoadItemNum(item);
  GameShopItem gst = ShopGameInstance::LoadShopItem((*item).id);
  gst.item = item;
  auto slot_max = 1;
  if (ItemGameInstance::CheckItem(item->id)) {
    slot_max = ItemGameInstance::LoadSlotMax(item->id);
  }
  switch (item->type) {
  case ItemEnum::kConsume:
  case ItemEnum::kEtc: {
    for (auto &shop : must_) {
      auto itm_id = shop.item->id;
      if (item->id == itm_id) {
        auto itm_num = ItemGameInstance::LoadItemNum(shop.item);
        auto num = std::min((int)add_num + itm_num, slot_max);
        ItemGameInstance::AddItemNum(shop.item, num - itm_num);
        add_num = add_num - (num - itm_num);
      }
    }
    while (add_num > 0) {
      auto num = std::min((int)add_num, slot_max);
      auto itm_num = ItemGameInstance::LoadItemNum(gst.item);
      ItemGameInstance::AddItemNum(gst.item, (num - itm_num));
      must_.push_back(gst);
      add_num = add_num - (num);
    }
    break;
  }
  default: {
    must_.push_back(gst);
    break;
  }
  }
  return true;
}

std::vector<std::polymorphic<GameItem> *>
ShopGameInstance::LoadPkgItems(int tab) {
  std::vector<std::polymorphic<GameItem> *> items;
  auto &r = PackageGameInstance::data_[tab];
  for (auto &itm : r) {
    if (!itm->id.empty()) {
      items.push_back(&itm);
    }
  }
  return items;
}

}  // namespace sdlms
