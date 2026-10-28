#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_TRAP_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_TRAP_GAME_INSTANCE_H_

#include "src/client/game/game_obj.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <array>
#include <cstdint>
#include <flat_map>
namespace sdlms {

class TrapGameInstance {
 public:
  static void Load(wz::Node* image);
  static void Load(uint32_t map_id);
  static inline std::array<std::flat_multimap<int64_t, GameObj>, 8> data_;

  struct AnimateData {
    int index;
    int time;
  };
  static AnimateData LoadAnimateIndex(std::vector<int> delays);
  static std::vector<int> LoadAnimateDelays(wz::Node* node);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_TRAP_GAME_INSTANCE_H_
