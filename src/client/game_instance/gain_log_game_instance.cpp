#include "gain_log_game_instance.h"
#include "src/client/window/window.h"
namespace sdlms {


void GainLogGameInstance::Add(const std::u16string &id, int num,
                                 GainEnum type) {
  GameGainLog g_log{
      .id = id,
      .num = num,
      .destroy = Window::dt_now_ + 10000,
      .type = type,
  };
  GainLogGameInstance::data_.emplace_back(g_log);
}

}  // namespace sdlms
