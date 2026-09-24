#include "gain_log_game_instance.h"
#include "src/client/window/window.h"

void gain_log_game_instance::add(const std::u16string &id, int num,
                                 gain_enum type) {
  game_gain_log g_log{
      .id = id,
      .num = num,
      .destroy = window::dt_now + 5000,
      .order = static_cast<uint8_t>(data.size()),
      .type = gain_enum::item,
  };
  gain_log_game_instance::data.emplace_back(g_log);
}