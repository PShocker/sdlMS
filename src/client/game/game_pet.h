#ifndef SDLMS_SRC_CLIENT_GAME_GAME_PET_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_PET_H_

#include <SDL3/SDL_rect.h>
#include "src/client/game/game_character.h"
#include <string>
namespace sdlms {


struct GamePet{
  GameCharacter *g_character;

  std::u16string action;

  int rx0;
  int rx1;

  SDL_FPoint pos;
  int64_t z;
  // 传送门
  uint32_t ani_index = 0;
  uint32_t ani_time;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_PET_H_
