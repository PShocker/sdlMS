#include "gain_log_logic_system.h"
#include "src/client/game/game_gain_log.h"
#include "src/client/game_instance/gain_log_game_instance.h"
#include "src/client/window/window.h"

void gain_log_logic_system::run_destroy() {
  auto &v = gain_log_game_instance::data;
  std::erase_if(v, [](game_gain_log &g_log) {
    auto destroy = g_log.destroy;
    return destroy <= window::dt_now;
  });
}

bool gain_log_logic_system::run() {
  run_destroy();
  return true;
}