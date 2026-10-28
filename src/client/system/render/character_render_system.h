#pragma once

#include "SDL3/SDL_pixels.h"
#include "src/client/game/game_character.h"
#include <cstdint>
namespace sdlms {

class CharacterRenderSystem {
private:
  static bool RenderAfterimage(GameCharacter &g_character);
  static void RenderTomb(GameCharacter &g_character);
  static bool RenderEffectFront(GameCharacter &g_character);
  static bool RenderEffectBack(GameCharacter &g_character);
  static void RenderChair(GameCharacter &g_character);
  static void RenderChair(GameCharacter &g_character, bool front);
  static void RenderMorph(GameCharacter &g_character);

public:
  static void RenderChatballoon(GameCharacter &g_character);
  static void RenderNametag(GameCharacter &g_character);
  static bool Render(GameCharacter &g_character);
  static bool RenderCharacter(GameCharacter &g_character);
};

}  // namespace sdlms
