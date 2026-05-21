#include "effect_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <optional>
#include <string>

void effect_game_instance::reset() { data = {}; }
