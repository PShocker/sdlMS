#pragma once

#include <cstdint>
#include <string>

enum class GainEnum { kItem, kExperience };

class GameGainLog {
public:
  std::u16string id;
  int32_t num;
  uint64_t destroy;

  GainEnum type;
};