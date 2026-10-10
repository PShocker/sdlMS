#include "gain_log_game_instance.h"
#include "src/client/window/window.h"

void gain_log_game_instance::add(const std::u16string &id, int num,
                                 gain_enum type) {
  game_gain_log g_log{
      .id = id,
      .num = num,
      .destroy = window::dt_now + 10000,
      .type = type,
  };
  gain_log_game_instance::data.emplace_back(g_log);
}