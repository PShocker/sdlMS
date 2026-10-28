#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_AFTERIMAGE_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_AFTERIMAGE_GAME_INSTANCE_H_

#include "src/client/game/game_character.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>

#include <cstdint>
#include <optional>
#include <string>
namespace sdlms {


struct AfterimageData {
  SDL_Texture* texture;
  SDL_FPoint origin;
  uint8_t alpha;
};

class AfterimageGameInstance {
 private:
  static wz::Node* LoadAtfNodeCache(const std::u16string& i);
  static wz::Node* LoadAtfNode(GameCharacter& g_character);

 public:
  static std::u16string LoadHitType(GameCharacter& g_character);
  static std::optional<AfterimageData> LoadData(GameCharacter& g_character);
  static std::optional<SDL_FRect> LoadRect(GameCharacter& g_character);
  static uint64_t LoadBeatTime(GameCharacter& g_character);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_AFTERIMAGE_GAME_INSTANCE_H_
