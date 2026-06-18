#include "game_save_system_instance.h"
#include "SDL3/SDL_filesystem.h"
#include "src/client/game/game_save.h"
#include <cassert>
#include <cstdlib>

game_save game_save_system_instance::load_save(const std::string &login) {
  game_save save;
  if (!SDL_CreateDirectory("./Save")) {
    assert(0);
    std::abort();
  }
  return save;
}