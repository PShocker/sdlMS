#include "backgrnd_logic_system.h"
#include "src/client/game_instance/backgrnd_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <ranges>
#include <string>

void BackgrndLogicSystem::RunAnimate(GameBackgrnd &g_backgrnd) {
  if (g_backgrnd.ani == 1) {
    auto texture_node = WzResource::map->find(g_backgrnd.path);
    auto ani_index = g_backgrnd.ani_index % texture_node->children_count();
    texture_node = texture_node->get_child(std::to_string(ani_index));

    auto delay =
        static_cast<wz::Property<int> *>(texture_node->get_child(u"delay"))
            ->get();

    g_backgrnd.ani_time += Window::delta_time;
    if (g_backgrnd.ani_time >= delay) {
      g_backgrnd.ani_time = 0;
      g_backgrnd.ani_index = 0;
    }
  }
}

void BackgrndLogicSystem::RunOffset(GameBackgrnd &g_backgrnd) {
  auto viewprot_x = CameraGameInstance::camera.x;
  auto viewprot_y = CameraGameInstance::camera.y;
  auto viewprot_w = CameraGameInstance::camera.w;
  auto viewprot_h = CameraGameInstance::camera.h;

  if (g_backgrnd.hspeed) {
    g_backgrnd.offset_x = fmodf(
        g_backgrnd.offset_x + g_backgrnd.rx * 5 * Window::delta_time / 1000.0f,
        g_backgrnd.cx);
  } else {
    g_backgrnd.offset_x =
        (viewprot_x + viewprot_w / 2) * (g_backgrnd.rx + 100) / 100.0;
  }
  if (g_backgrnd.vspeed) {
    g_backgrnd.offset_y = fmodf(
        g_backgrnd.offset_y + g_backgrnd.ry * 5 * Window::delta_time / 1000.0,
        g_backgrnd.cy);
  } else {
    g_backgrnd.offset_y =
        (viewprot_y + viewprot_h / 2) * (g_backgrnd.ry + 100) / 100.0;
  }
}

bool BackgrndLogicSystem::Run() {
  for (auto m :
       {&BackgrndGameInstance::front, &BackgrndGameInstance::back}) {
    for (auto &backgrnd : *m | std::views::values) {
      RunOffset(backgrnd);
      RunAnimate(backgrnd);
    }
  }
  return true;
}