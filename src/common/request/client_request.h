#pragma once

#include <cstdint>
class client_request {
public:
  static void client_heartbeat_request();
  static void client_in_scene_request();
  static void client_move_request(float x1, float x2, float y1, float y2,
                                  uint32_t time);
};