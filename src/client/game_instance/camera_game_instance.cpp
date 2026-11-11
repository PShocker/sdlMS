#include "src/client/game_instance/camera_game_instance.h"

#include "src/client/window/window.h"

#include "src/client/game_instance/character_game_instance.h"

namespace sdlms {

void CameraGameInstance::Shake(float power, uint64_t ms) {
  shake_power_ = power;
  shake_end_ = Window::dt_now_ + ms;
}

void CameraGameInstance::Reset() {
  const auto& self = CharacterGameInstance::self_;
  camera_.x = self.pos.x - camera_.w / 2;
  camera_.y = self.pos.y - camera_.h / 2;
}

void CameraGameInstance::Load(float x, float y, float w, float h) {
  camera_.x = x;
  camera_.y = y;
  camera_.w = w;
  camera_.h = h;
}

}  // namespace sdlms
