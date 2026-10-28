#ifndef SDLMS_SRC_CLIENT_GAME_GAME_BALL_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_BALL_H_

#include <SDL3/SDL.h>
#include <cstdint>
#include <string>
namespace sdlms {


struct GameBall{
  std::u16string path;

  SDL_FPoint goal;
  uint32_t mob_index;

  uint64_t delay;
  int speed;

  SDL_FPoint pos;
  bool flip;
  int32_t rotate = 0;

  uint32_t ani_index = 0;
  uint32_t ani_time = 0;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_BALL_H_
