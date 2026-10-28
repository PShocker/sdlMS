#pragma once

#include "src/client/game/game_obj.h"
class ObjLogicSystem {
public:
  static void run_animate(GameObj &g_obj);
  static void run_alpha(GameObj &g_obj);
  static bool run();
};