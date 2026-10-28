#include "src/client/game_instance/drop_game_instance.h"
#include <SDL3/SDL_rect.h>
#include "src/client/game/game_drop.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include <format>
#include <optional>
namespace sdlms {


void DropGameInstance::Reset() { data_ = {}; }

}  // namespace sdlms
