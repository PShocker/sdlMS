#pragma once

#include <cstdint>
#include <string>

enum class gain_enum { item, experience };

class game_gain_log {
public:
  std::u16string id;
  uint64_t num;
  uint64_t destroy;

  gain_enum type;
};