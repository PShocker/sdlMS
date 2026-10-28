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
  static wz::Node *load_atf_node_cache(const std::u16string &i);
  static wz::Node *load_atf_node(GameCharacter &g_character);

public:
  static std::u16string load_hit_type(GameCharacter &g_character);
  static std::optional<AfterimageData> load_data(GameCharacter &g_character);
  static std::optional<SDL_FRect> load_rect(GameCharacter &g_character);
  static uint64_t load_beat_time(GameCharacter &g_character);
};