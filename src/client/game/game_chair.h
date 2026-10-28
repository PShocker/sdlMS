#ifndef SDLMS_SRC_CLIENT_GAME_GAME_CHAIR_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_CHAIR_H_

#include <SDL3/SDL.h>
#include <cstdint>
#include <optional>
#include <string>
namespace sdlms {


struct GameChair{
  std::u16string id;
  uint8_t ani_index = 0;
  uint32_t ani_time = 0;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_CHAIR_H_
