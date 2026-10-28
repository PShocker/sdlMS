#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_item_buff.h"
#include "wz/Node.h"
#include <cstdint>
#include <memory>
#include <string>
class ItemGameInstance {
public:
  static inline std::vector<GameItemBuff> buff_;

  static bool CheckItem(const std::u16string &id);

  static std::u16string LoadItemText(const std::u16string &id,
                                       const std::u16string &val);
  static std::u16string LoadItemType(const std::u16string &id);
  static wz::Node *LoadItemInfo(const std::u16string &id, uint32_t num);
  static int LoadSlotMax(const std::u16string &id);
  static std::polymorphic<GameItem> LoadItem(const std::u16string &id,
                                               uint32_t num);
  static int LoadItemNum(std::polymorphic<GameItem> &itm);
  static void AddItemNum(std::polymorphic<GameItem> &item, int num);
  static void DecItemNum(std::polymorphic<GameItem> &item, int num);
  static void SetItemNum(std::polymorphic<GameItem> &itm, int num);

  static void UseMorphItem(const std::u16string &id, GameCharacter &g);
  static void UnuseMorphItem(const std::u16string &id, GameCharacter &g);
  static bool UseMobItem(std::polymorphic<GameItem> &itm);
  static bool UseBuffItem(std::polymorphic<GameItem> &itm);
  static void UnuseBuffItem(const std::u16string &id);
  static bool UseConsumeItem(const std::u16string &id);
  static bool UseConsumeItem(std::polymorphic<GameItem> &itm);
  static void UseItem(std::polymorphic<GameItem> &itm);
};