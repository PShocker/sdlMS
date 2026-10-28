#ifndef SDLMS_SRC_CLIENT_GAME_GAME_INPUT_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_INPUT_H_

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <string>
#include <vector>
namespace sdlms {


struct GameInput {
  std::string type;
  std::string val;
  std::string sub_val;

  auto operator<=>(const GameInput&) const = default;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_INPUT_H_
