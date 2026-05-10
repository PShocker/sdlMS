#include "camera_game_instance.h"
#include "character_game_instance.h"

void camera_game_instance::reset() {
  const auto &self = character_game_instance::self;
  camera.x = self.pos.x - camera.w / 2;
  camera.y = self.pos.y - camera.h / 2;
}

void camera_game_instance::load(float x, float y, float w, float h) {
  camera.x = x;
  camera.y = y;
  camera.w = w;
  camera.h = h;
}