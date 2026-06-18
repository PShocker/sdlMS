#include "game_save_system_instance.h"
#include "SDL3/SDL_filesystem.h"
#include "src/client/game/game_save.h"
#include <cassert>
#include <cstdlib>
#include <string>

bool game_save_system_instance::load_save(const std::string &login) {
  if (!SDL_CreateDirectory("./Save")) {
    assert(0);
    std::abort();
  }
  size_t file_size = 0;
  std::string path = "./Save/" + login + ".bin";
  void *data = SDL_LoadFile(path.c_str(), &file_size);
  if (data != nullptr) {

    SDL_free(data);
    return true;
  }
  save = {};
  return false;
}