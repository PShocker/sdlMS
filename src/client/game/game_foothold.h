#pragma once

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>

namespace sdlms {

struct GameFoothold {
  uint32_t id;
  uint32_t page;
  uint32_t zmass;
  int32_t prev;
  int32_t next;

  int32_t x1;
  int32_t y1;
  int32_t x2;
  int32_t y2;

  int32_t l;
  int32_t r;
  int32_t t;
  int32_t b;

  bool forbid_fall_down = false;
  bool cant_through = false;

  // 095 CAttrFoothold：walk=地面速度倍率（默认 1.0）、drag=地面推力倍率
  // （默认 1.0）、force=传送带恒力（默认 0）。WZ 中 drag/force 为百分数。
  float walk = 1.0f;
  float drag = 1.0f;
  float force = 0.0f;

  std::optional<float> k = std::nullopt;  // 斜率,若k值不存在,可以判断是否是墙面
  std::optional<float> intercept = std::nullopt;
};

}  // namespace sdlms
