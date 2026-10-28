#pragma once

#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_character.h"
#include "wz/Node.h"
#include <cstdint>
#include <optional>
#include <string>

struct AfterimageData {
  SDL_Texture *texture;
  SDL_FPoint origin;
  uint8_t alpha;
};

class AfterimageGameInstance {
private:
  static wz::Node *LoadAtfNodeCache(const std::u16string &i);
  static wz::Node *LoadAtfNode(GameCharacter &g_character);

public:
  static std::u16string LoadHitType(GameCharacter &g_character);
  static std::optional<AfterimageData> LoadData(GameCharacter &g_character);
  static std::optional<SDL_FRect> LoadRect(GameCharacter &g_character);
  static uint64_t LoadBeatTime(GameCharacter &g_character);
};