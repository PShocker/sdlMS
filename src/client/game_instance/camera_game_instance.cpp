#include "camera_game_instance.h"
#include "character_game_instance.h"

void CameraGameInstance::reset() {
  const auto &self = CharacterGameInstance::self;
  camera.x = self.pos.x - camera.w / 2;
  camera.y = self.pos.y - camera.h / 2;
}

void CameraGameInstance::load(float x, float y, float w, float h) {
  camera.x = x;
  camera.y = y;
  camera.w = w;
  camera.h = h;
}