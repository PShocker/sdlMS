#pragma once

#include <cstdint>
class scene_system_instance {
public:
  static inline uint32_t map_id;

  static void enter(uint32_t map_id);
};