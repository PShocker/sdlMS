#include "shop_game_instance.h"
#include "src/client/game/game_shop.h"
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
    auto itemId =
        static_cast<wz::Property<std::u16string> *>(node->get_child(u"itemid"))
            ->get();
    game_shop_item item{
        .itemId = itemId,
        .price = price,
    };
    shop.items.push_back(item);
  }
  return shop;
}

std::optional<game_shop>
shop_game_instance::load_npc_shop(const std::u16string &npc_id) {
  std::optional<game_shop> shop;
  static std::flat_map<std::u16string, game_shop> cache;
  if (cache.empty()) {
    auto node = wz_resource::ms->get_root()->find(u"Shop.img/ShopNpc");
    for (auto [k, v] : *node->get_children()) {
      auto npc_i = static_cast<wz::Property<std::u16string> *>(v[0])->get();
      game_shop gs = load_shop(k);
      cache[npc_i] = gs;
    }
  }
  if (cache.contains(npc_id)) {
    return cache.at(npc_id);
  } else {
    return std::nullopt;
  }
}