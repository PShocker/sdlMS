#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_CAMERA_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_CAMERA_GAME_INSTANCE_H_

#include <SDL3/SDL_rect.h>

namespace sdlms {

class CameraGameInstance {
 public:
  static void Reset();
  static void Load(float x, float y, float w, float h);
  static inline SDL_FRect camera_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_CAMERA_GAME_INSTANCE_H_
