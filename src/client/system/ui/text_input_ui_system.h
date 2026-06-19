#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include <string>

struct text_input {
  int max_size;
  std::u16string text;
  std::u16string composition;
  bool disable = false;
  bool active = false;
  SDL_Rect r;

  SDL_Color color;
  int font_size;
  int cur = 0;
};

class text_input_ui_system {
public:
  static void render(text_input &input, int x, int y);
  static void active(text_input &input);
  static bool event(SDL_Event *event, text_input &input);
};