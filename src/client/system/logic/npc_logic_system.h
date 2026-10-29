#pragma once

#include "src/client/game/game_npc.h"

#include <SDL3/SDL_events.h>

#include <optional>
#include <string>

namespace sdlms {

class NpcLogicSystem {
 private:
  static bool RunAnimate(GameNpc& g_npc);
  static bool RunMove(GameNpc& g_npc);
  static void UpdateChat(GameNpc& g_npc);

  static void EnterStand(GameNpc& g_npc);
  static void EnterAction(GameNpc& g_npc, const std::u16string& action);
  static void EnterMove(GameNpc& g_npc);
  static void ChooseNext(GameNpc& g_npc);
  static void ResetAnimate(GameNpc& g_npc);

 public:
  static bool Run();
};

}  // namespace sdlms
