#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <optional>

class game_ball {
private:
  std::optional<SDL_FPoint> target_point = std::nullopt;
  std::optional<SDL_FPoint> point = std::nullopt;
  // 摧毁时间
  uint64_t destory;
  int32_t rotate = 0;
  // 是否需要改变轨迹跟踪怪物
  bool track = true;
  std::optional<std::unordered_set<entt::entity>> track_hit = std::nullopt;
};