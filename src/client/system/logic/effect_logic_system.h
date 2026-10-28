#ifndef SDLMS_SRC_CLIENT_SYSTEM_LOGIC_EFFECT_LOGIC_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_LOGIC_EFFECT_LOGIC_SYSTEM_H_

#include "src/client/game/game_effect.h"

#include <vector>

namespace sdlms {

class EffectLogicSystem {
 private:
  static bool RunDamage(GameEffect& g_effect);
  static bool RunSkillAffected(GameEffect& g_effect);
  static bool RunSkillHit(GameEffect& g_effect);
  static bool RunSkillRegion(GameEffect& g_effect);

  static void RunAnimate(std::vector<GameEffect>& v);

 public:
  static bool RunEffect(GameEffect& g_effect);
  static bool RunSkillUse(GameEffect& g_effect);

  static bool Run();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_LOGIC_EFFECT_LOGIC_SYSTEM_H_
