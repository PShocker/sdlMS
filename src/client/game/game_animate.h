#ifndef SDLMS_SRC_CLIENT_GAME_GAME_ANIMATE_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_ANIMATE_H_

#include <cstdint>
#include <vector>
namespace sdlms {

struct GameAnimate{
  uint8_t ani_index = 0;
  uint32_t ani_time = 0;
  std::vector<uint32_t> ani_delay;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_ANIMATE_H_
