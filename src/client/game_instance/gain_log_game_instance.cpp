#include "gain_log_game_instance.h"
#include "src/client/game/game_gain_log.h"
#include "src/client/window/window.h"

void gain_log_game_instance::load_exp(uint64_t num) {
  game_gain_log g_log{
      .id = u"",
      .num = num,
      .destory = window::dt_now + 5000,
      .experience = true,
  };
  data.push_back(g_log);
}

void gain_log_game_instance::load_item(const std::u16string &id) {
  game_gain_log g_log{
      .id = id,
      .num = 1,
      .destory = window::dt_now + 5000,
      .experience = false,
  };
  data.push_back(g_log);
}
