#pragma once

#include "src/client/game/game_gain_log.h"
#include <cstdint>
#include <string>
#include <vector>
class GainLogGameInstance {
public:
  static inline std::vector<GameGainLog> data;

  static void add(const std::u16string &id, int num, GainEnum type);
};