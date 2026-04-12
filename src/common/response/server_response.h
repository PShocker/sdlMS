#pragma once

#include <cstdint>
class server_response {
public:
  static void server_heartbeat_response(uint64_t client_id);
  static void server_in_scene_response();
  static void server_move_response(uint64_t client_id, float x1, float x2,
                                   float y1, float y2, uint32_t time);
};