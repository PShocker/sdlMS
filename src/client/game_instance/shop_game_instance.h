#pragma once
#include "src/client/game/game_shop.h"
#include <cstdint>
#include <optional>
#include <string>

class ShopGameInstance {
public:
  static std::polymorphic<GameItem> load_item(const std::u16string &id);
  static const GameShop *load_npc_shop(const std::u16string &npc_id);
  static GameShop load_shop(const std::u16string &shop_id);
  static GameShopItem load_shop_item(const std::u16string &id);

  static inline std::vector<GameShopItem> must;
  static bool add_must_item(std::polymorphic<GameItem> &item);
  static std::vector<std::polymorphic<GameItem> *> load_pkg_items(int tab);
};