#pragma once

#include <SDL3/SDL_events.h>

#include <functional>
#include <vector>

namespace sdlms {

class System {
 public:
  static inline std::vector<bool (*)()> render_systems_;
  static inline std::vector<bool (*)()> logic_systems_;
  static inline std::vector<bool (*)(SDL_Event*)> event_systems_;
};

}  // namespace sdlms
