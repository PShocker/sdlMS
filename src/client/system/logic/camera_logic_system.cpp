#include "camera_logic_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include <cmath>

bool camera_logic_system::run() {
  auto self = character_game_instance::self;
  auto self_pos = self.pos;
  auto &camera = camera_game_instance::camera;

  auto prev_x = camera.x;
  auto next_x = self_pos.x - camera.w / 2; // 人物移动后新的摄像机坐标
  auto delta_x = next_x - prev_x;

  camera.x =
      std::roundf(std::lerp(prev_x, next_x, std::abs(delta_x) / 6000.0f));

  auto prev_y = camera.y;
  auto next_y = self_pos.y - camera.h / 2; // 人物移动后新的摄像机坐标
  auto delta_y = next_y - prev_y;

  camera.y =
      std::roundf(std::lerp(prev_y, next_y, std::abs(delta_y) / 6000.0f));

  auto border =
      map_info_game_instance::load_vr_border(scene_system_instance::map_id);
  auto [t, l, b, r] = border;
  if (camera.x < l) {
    camera.x = l;
  } else if (camera.x + camera.w > r) {
    camera.x = r - camera.w;
  }
  if (camera.y < t) {
    camera.y = t;
  } else if (camera.y + camera.h > b) {
    camera.y = b - camera.h;
  }
  if (r - l < camera.w) {
    // 如果地图宽度小于摄像机宽度,地图居中显示
    camera.x = l - (camera.w - (r - l)) / 2;
  }
  if (b - t < camera.h) {
    // 如果地图高度小于摄像机高度,地图居中显示
    camera.y = t - (camera.h - (b - t)) / 2;
  }

  return true;
}