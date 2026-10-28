#include "src/client/game_instance/effect_game_instance.h"

#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/common/flatbuffers/common.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>

namespace sdlms {

void EffectGameInstance::Reset() {
  data_ = {};
}

}  // namespace sdlms
