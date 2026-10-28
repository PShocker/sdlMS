#ifndef SDLMS_SRC_CLIENT_GAME_GAME_REGION_SKILL_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_REGION_SKILL_H_

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <string>
namespace sdlms {


struct GameRegionSkill {
  std::u16string id;
  uint8_t lv;
  SDL_FPoint pos;
  bool flip;

  uint64_t start;
  uint64_t end;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_REGION_SKILL_H_
