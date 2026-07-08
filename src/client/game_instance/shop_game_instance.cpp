#include "shop_game_instance.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_shop.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <flat_map>
#include <optional>
#include <string>

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
    game_shop_item item{
        .price = price,
    };
    if (item_game_instance::check_item(id)) {
      game_item itm;
      itm.id = id;
      item.item = std::make_unique<game_item>(itm);
    } else {
      game_equip_item eqp;
      eqp.id = id;
      item.item = std::make_unique<game_equip_item>(eqp);
    }
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