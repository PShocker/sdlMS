#ifndef SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_SCENE_SYSTEM_INSTANCE_H_
#define SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_SCENE_SYSTEM_INSTANCE_H_

#include <SDL3/SDL_rect.h>
#include "src/client/game/game_character.h"
#include <cstdint>
#include <optional>
#include <string>
namespace sdlms {

class SceneSystemInstance {
public:
  static inline uint32_t map_id_;
  static inline uint32_t prepare_map_id_;
  static inline std::optional<SDL_FPoint> prepare_pos_;
  static void Enter(uint32_t map_id);
  static void EnterFade();
  static void EnterPrepare(uint32_t map_id, const std::u16string &pn, int i);
  static void EnterQuit();
  static void QuitPrepare();
  static void EnterCashShop();
  static void CashShopPrepare();

private:
  static SDL_FPoint LoadPreparePos(const std::u16string &pn, uint8_t index);
  static void EnterPrepare();
  static bool RenderGame();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_SCENE_SYSTEM_INSTANCE_H_
