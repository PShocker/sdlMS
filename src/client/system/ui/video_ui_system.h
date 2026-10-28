#pragma once

#include "SDL3/SDL_rect.h"
#include <string>
class VideoUiSystem {
public:
  static bool render();
  static void open(const std::string &path);
  static void close();
};