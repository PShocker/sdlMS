#pragma once
#include "SDL3/SDL_audio.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_save.h"
#include <array>
#include <cstdint>
#include <optional>
class package_game_instance {
public:
  // enum class package_enum { equip, consume, etc, install, cash ,deco};
  static inline std::array<std::vector<std::polymorphic<game_item>>, 6> data;

  static inline uint64_t meso = UINT64_MAX;

  static void load(const character_save &cs);

  static std::vector<int> load_empty_index(uint32_t tab);
  static std::vector<int> load_empty_index(std::polymorphic<game_item> &item);

  static std::polymorphic<game_item> *load_active_ball();
  static std::u16string load_active_cash_ball();
  static uint32_t load_item_num(const std::u16string &id);
  static std::polymorphic<game_item> *load_item(const std::u16string &id);

  static int add_item_slot(std::polymorphic<game_item> &item, int i);
  static std::optional<int> add_item(std::polymorphic<game_item> &item);
};