#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>

namespace sdlms {

struct GameBall {
  std::u16string path;

  // goal 是相对目标怪物的终点偏移；没有目标时是绝对终点。
  SDL_FPoint goal;
  uint32_t mob_index;

  uint64_t delay;
  int speed;

  SDL_FPoint pos;
  bool flip;
  int32_t rotate = 0;

  // 095：子弹 origin 绑定目标 Vector2D，但起点/终点偏移在发射时固定。
  // 这样怪物移动时整条弹道平移，不会每帧重新瞄准怪物中心。
  SDL_FPoint start{};
  SDL_FPoint start_offset{};
  SDL_FPoint spawn_goal{};
  uint64_t move_start = 0;
  uint64_t duration = 0;

  uint32_t ani_index = 0;
  uint32_t ani_time = 0;
};

}  // namespace sdlms
