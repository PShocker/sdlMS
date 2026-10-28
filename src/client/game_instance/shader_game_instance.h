#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_SHADER_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_SHADER_GAME_INSTANCE_H_

#include <SDL3/SDL_render.h>

#include <flat_map>
#include <vector>

namespace sdlms {

enum class ShaderEnum {
  kGrayscale,
  kOutline,
};

class ShaderGameInstance {
 public:
  static bool Init();
  static void Start(ShaderEnum type);
  static void End();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_SHADER_GAME_INSTANCE_H_
