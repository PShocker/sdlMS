#pragma once

#include "SDL3/SDL_pixels.h"
#include "src/client/game/game_character.h"
#include <cstdint>
class CharacterRenderSystem {
private:
  static bool render_afterimage(GameCharacter &g_character);
  static void render_tomb(GameCharacter &g_character);
  static bool render_effect_front(GameCharacter &g_character);
  static bool render_effect_back(GameCharacter &g_character);
  static void render_chair(GameCharacter &g_character);
  static void render_chair(GameCharacter &g_character, bool front);
  static void render_morph(GameCharacter &g_character);

public:
  static void render_chatballoon(GameCharacter &g_character);
  static void render_nametag(GameCharacter &g_character);
  static bool render(GameCharacter &g_character);
  static bool render_character(GameCharacter &g_character);
};