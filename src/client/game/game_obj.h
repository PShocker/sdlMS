#pragma once

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>

namespace sdlms {

// 095 CMapLoadable::MakeVectorAnimate：obj 帧的向量运动宏。
// moveType 0=MakeRotate 绕 (moveP 周期) 旋转（moveR 优先）
//         1=MakeOscilate 横向往复（moveW 振幅，moveP 周期）
//         2=MakeOscilate 纵向往复（moveH 振幅）
//         3=MakeElipticalMove 椭圆（moveW/moveH 双振幅，moveP 走完一圈
//           的周期，方向由 moveP 正负决定）
// 位置 = 基点 + 宏在当前时间的位移（相位取全局时钟，不同客户端一致，
// 陷阱位置由服务器/系统时间统一同步）。
struct GameObjMove {
  int32_t type = -1;        // moveType；-1 = 静态摆设
  int32_t w = 0;            // moveW 振幅（默认 100）
  int32_t h = 0;            // moveH 振幅（默认 100）
  int32_t p = 5000;         // moveP 周期 ms（默认 5000）
  int32_t r = 0;            // moveR 旋转周期 ms（MoveRotate 用）
  int32_t center_start = 0; // centerStart 相位偏移（0/1）
};

struct GameObj {
  std::u16string path;
  int64_t z;

  SDL_FPoint pos;
  int8_t flip;

  uint8_t alpha = 255;

  uint32_t ani_index = 0;
  uint32_t ani_time;
  uint8_t ani_count;

  // 陷阱（obj_trap）运行时状态：base 是 WZ 摆放坐标（move 宏的原点），
  // pos 每帧按服务器时间的相位重算。
  GameObjMove move;
  SDL_FPoint base{};
  // 帧动画的周期总长（各帧 delay 之和），相位按全局时钟取模，
  // 保证不同客户端在同一时刻显示同一帧。
  uint32_t ani_cycle = 0;
  uint64_t ani_start_time = 0;
};

}  // namespace sdlms
