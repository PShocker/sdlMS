#pragma once

#include <any>
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
class map_info_game_instance {
public:
  static inline std::flat_map<std::u16string, std::any> map_info;
  static void load(uint32_t map_id);
};