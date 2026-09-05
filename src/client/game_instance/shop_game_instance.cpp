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

std::polymorphic<game_item>
shop_game_instance::load_item(const std::u16string &id) {
  std::polymorphic<game_item> r;
  if (item_game_instance::check_item(id)) {
    r = item_game_instance::load_item(id, 0);
  } else {
    game_equip_item eqp;
    eqp.type = item_enum::equip;
    eqp.id = id;
    r = std::polymorphic<game_item>(std::in_place_type<game_equip_item>, eqp);
  }
  return r;
}

game_shop_item shop_game_instance::load_shop_item(const std::u16string &id) {
  auto item = shop_game_instance::load_item(id);
  game_shop_item gst;
  if (item->type == item_enum::equip) {
    auto info = equip_game_instance::load_equip_info(item->id);
    gst.price =
        static_cast<wz::Property<int> *>(info->get_child(u"price"))->get();
  } else {
    auto info = item_game_instance::load_item_info(item->id, 0);
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

game_shop shop_game_instance::load_shop(const std::u16string &shop_id) {
  game_shop shop;
  auto node =
      wz_resource::ms->get_root()->find(u"Shop.img/ShopItem/" + shop_id);
  for (auto [k, v] : *node->get_children()) {
    auto node = v[0];
    auto price =
        static_cast<wz::Property<int> *>(node->get_child(u"price"))->get();
    auto id =
        static_cast<wz::Property<std::u16string> *>(node->get_child(u"itemid"))
            ->get();
    game_shop_item item;
    item.price = price;
    item.item = load_item(id);
    shop.items.push_back(std::move(item));
  }
  return shop;
}

const game_shop *
shop_game_instance::load_npc_shop(const std::u16string &npc_id) {
  std::optional<game_shop> shop;
  static std::flat_map<std::u16string, game_shop> cache;
  if (cache.empty()) {
    auto node = wz_resource::ms->get_root()->find(u"Shop.img/ShopNpc");
    for (auto [k, v] : *node->get_children()) {
      auto npc_i = static_cast<wz::Property<std::u16string> *>(v[0])->get();
      cache.emplace(std::move(npc_i), load_shop(k));
    }
  }
  if (cache.contains(npc_id)) {
    return &cache.at(npc_id);
  } else {
    return nullptr;
  }
}

bool shop_game_instance::add_must_item(std::polymorphic<game_item> &item) {
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
        item_game_instance::add_item_num(shop.item, num - itm_num);
        add_num = add_num - (num - itm_num);
      }
    }
    while (add_num > 0) {
      auto num = std::min((int)add_num, slot_max);
      auto itm_num = item_game_instance::load_item_num(gst.item);
      item_game_instance::add_item_num(gst.item, (num - itm_num));
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

std::vector<std::polymorphic<game_item> *>
shop_game_instance::load_pkg_items(int tab) {
  std::vector<std::polymorphic<game_item> *> items;
  auto &r = package_game_instance::data[tab];
  for (auto &itm : r) {
    if (!itm->id.empty()) {
      items.push_back(&itm);
    }
  }
  return items;
}