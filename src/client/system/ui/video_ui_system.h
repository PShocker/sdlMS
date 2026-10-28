#pragma once

#include "SDL3/SDL_rect.h"
#include <string>
class VideoUiSystem {
public:
  static bool Render();
  static void Open(const std::string &path);
  static void Close();
};