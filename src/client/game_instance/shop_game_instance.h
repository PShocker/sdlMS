#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_SHOP_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_SHOP_GAME_INSTANCE_H_

#include "src/client/game/game_shop.h"

#include <cstdint>
#include <optional>
#include <string>

namespace sdlms {

class ShopGameInstance {
 public:
  static std::polymorphic<GameItem> LoadItem(const std::u16string& id);
  static const GameShop* LoadNpcShop(const std::u16string& npc_id);
  static GameShop LoadShop(const std::u16string& shop_id);
  static GameShopItem LoadShopItem(const std::u16string& id);

  static inline std::vector<GameShopItem> must_;
  static bool AddMustItem(std::polymorphic<GameItem>& item);
  static std::vector<std::polymorphic<GameItem>*> LoadPkgItems(int tab);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_SHOP_GAME_INSTANCE_H_
