#ifndef SDLMS_SRC_CLIENT_SYSTEM_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_SYSTEM_H_

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

#endif  // SDLMS_SRC_CLIENT_SYSTEM_SYSTEM_H_
