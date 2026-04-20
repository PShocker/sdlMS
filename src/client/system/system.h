#pragma once

#include "SDL3/SDL_events.h"
#include <functional>
#include <vector>
class system {
public:
  static inline std::vector<bool (*)()> render_systems;
  static inline std::vector<bool (*)()> logic_systems;
  static inline std::vector<bool (*)(SDL_Event *)> event_systems;
};