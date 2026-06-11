#include "gain_log_logic_system.h"
#include "src/client/game/game_gain_log.h"
#include "src/client/game_instance/gain_log_game_instance.h"
#include "src/client/window/window.h"

bool gain_log_logic_system::run() {
  auto &v = gain_log_game_instance::data;

  std::erase_if(v, [](game_gain_log &g_log) {
    auto destory = g_log.destory;
    return destory <= window::dt_now;
  });
  return true;
}