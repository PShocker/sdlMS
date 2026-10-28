#pragma once
#include "SDL3/SDL_audio.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_save.h"
#include <array>
#include <cstdint>
#include <optional>
class PackageGameInstance {
public:
  // enum class package_enum { equip, consume, etc, install, cash ,deco};
  static inline std::array<std::vector<std::polymorphic<GameItem>>, 6> data_;

  static inline uint64_t meso_ = UINT64_MAX;

  static void Load(const CharacterSaveData &cs);

  static std::vector<int> LoadEmptyIndex(uint32_t tab);
  static std::vector<int> LoadEmptyIndex(std::polymorphic<GameItem> &item);

  static std::polymorphic<GameItem> *LoadActiveBall();
  static std::u16string LoadActiveCashBall();
  static uint32_t LoadItemNum(const std::u16string &id);
  static std::polymorphic<GameItem> *LoadItem(const std::u16string &id);

  static int AddItemSlot(std::polymorphic<GameItem> &item, int i);
  static std::optional<int> AddItem(std::polymorphic<GameItem> &item);
  static std::optional<int> AddNewItem(std::polymorphic<GameItem> &item);
  static bool AddMeso(uint32_t num);
};