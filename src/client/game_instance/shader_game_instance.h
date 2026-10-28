#pragma once

#include "SDL3/SDL_render.h"
#include <flat_map>
#include <vector>
enum class ShaderEnum {
  kGrayscale,
  kOutline,
};

class ShaderGameInstance {
public:
  static bool init();
  static void start(ShaderEnum type);
  static void end();
};