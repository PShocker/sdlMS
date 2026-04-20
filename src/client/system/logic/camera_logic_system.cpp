#include "camera_logic_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include <cmath>

bool camera_logic_system::run() {
  // auto self = character_game_instance::self;
  // auto self_pos = self.pos;
  // auto &camera = camera_game_instance::camera;

  // auto prev_x = camera.x;
  // auto next_x = self_pos.x - camera.w / 2; // 人物移动后新的摄像机坐标
  // auto delta_x = next_x - prev_x;

  // camera.x =
  //     std::roundf(std::lerp(prev_x, next_x, std::abs(delta_x) / 6000.0f));

  // auto prev_y = camera.y;
  // auto next_y = self_pos.y - camera.h / 2; // 人物移动后新的摄像机坐标
  // auto delta_y = next_y - prev_y;

  // camera.y =
  //     std::roundf(std::lerp(prev_y, next_y, std::abs(delta_y) / 6000.0f));
  return true;
}