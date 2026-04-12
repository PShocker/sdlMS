#pragma once

#include <functional>
#include <vector>
class system {
  static inline std::vector<std::function<bool()>> render_systems;
  static inline std::vector<std::function<bool()>> logic_systems;
  static inline std::vector<std::function<bool()>> event_systems;
};