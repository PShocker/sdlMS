#pragma once

#include <cstdint>
class game_backgrnd {
private:
  std::variant<Sprite, AnimatedSprite> spr;
  // 平铺间隔
  int32_t cx = 0;
  int32_t cy = 0;
  // 是否平铺
  bool htile = false;
  bool vtile = false;

  // 平移属性
  int32_t rx = 0;
  int32_t ry = 0;
  float offset_x = 0.0f;
  float offset_y = 0.0f;
  bool hspeed = false;
  bool vspeed = false;
};