#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <optional>
#include <string>
class server_foothold {
public:
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

  std::optional<float> k = std::nullopt; // 斜率,若k值不存在,可以判断是否是墙面
  std::optional<float> intercept = std::nullopt;
};