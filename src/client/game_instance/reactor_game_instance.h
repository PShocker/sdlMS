#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_REACTOR_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_REACTOR_GAME_INSTANCE_H_
#include "src/client/game/game_reactor.h"
#include <array>
#include <cstdint>
#include <vector>
namespace sdlms {


class ReactorGameInstance {
public:
  static void Load(uint32_t map_id);
  static inline std::array<std::vector<GameReactor>, 8> data_;

private:
  static uint8_t LoadReactorLayer(SDL_FPoint pos);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_REACTOR_GAME_INSTANCE_H_
