#ifndef SDLMS_SRC_CLIENT_SYSTEM_LOGIC_MOB_LOGIC_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_LOGIC_MOB_LOGIC_SYSTEM_H_

#include "src/client/game/game_mob.h"

#include <string>

namespace sdlms {

class MobLogicSystem {
 private:
  static bool RunAnimate(GameMob& g_mob);
  static void RunCollision();
  static void RunMv();
  static void RunStateMachine(GameMob& g_mob);
  static void RunAlpha(GameMob& g_mob);
  static void RunDebuff(GameMob& g_mob);

 public:
  enum class ActionEnum {
    kStand,
    kJump,
    kMove,
    kSwim,
    kFly,
    kHit,
    kDie,
    kRevive,
  };
  static ActionEnum LoadActionType(const std::u16string& action);

  static bool Run();
  static std::optional<SDL_FRect> LoadRect(const GameMob& g_mob);
  static std::optional<SDL_FPoint> LoadHead(const GameMob& g_mob);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_LOGIC_MOB_LOGIC_SYSTEM_H_
