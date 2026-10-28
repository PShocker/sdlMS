#include "gain_log_game_instance.h"
#include "src/client/window/window.h"

void GainLogGameInstance::add(const std::u16string &id, int num,
                                 GainEnum type) {
  GameGainLog g_log{
      .id = id,
      .num = num,
      .destroy = Window::dt_now + 10000,
      .type = type,
  };
  GainLogGameInstance::data.emplace_back(g_log);
}