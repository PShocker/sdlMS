#pragma once

#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_character.h"
#include "wz/Node.h"
#include <cstdint>
#include <optional>
#include <string>

struct afterimage_data {
  SDL_Texture *texture;
  SDL_FPoint origin;
  uint8_t alpha;
};

struct afterimage_ltrb {
  SDL_FPoint lt;
  SDL_FPoint rb;
};
class afterimage_game_instance {
private:
  static wz::Node *load_atf_node_cache(const std::u16string &i);
  static wz::Node *load_atf_node(game_character &g_character);

public:
  static std::optional<afterimage_data> load_data(game_character &g_character);
  static std::optional<afterimage_ltrb> load_rect(game_character &g_character);
  static uint64_t load_beat_time(game_character &g_character);
};