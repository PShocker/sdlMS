#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_VIDEO_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_VIDEO_UI_SYSTEM_H_

#include <SDL3/SDL_rect.h>

#include <string>

namespace sdlms {

class VideoUiSystem {
 public:
  static bool Render();
  static void Open(const std::string& path);
  static void Close();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_VIDEO_UI_SYSTEM_H_
