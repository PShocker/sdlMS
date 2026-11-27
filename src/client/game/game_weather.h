#pragma once

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <cstdint>
#include <vector>

namespace sdlms {

// 095 CMapLoadable::TransientLayer_Weather 的单颗粒子：
// 屏幕坐标、速度（px/ms）、摆动与自转，均按服务器同步的动画时间推进。
struct GameWeatherParticle {
  wz::Node* sprite = nullptr;  // MapHelper.img/weather/<name>/<n>
  SDL_FPoint pos{};            // 屏幕坐标（含摆动偏移）
  SDL_FPoint origin{};         // 画布 origin
  float base_x = 0.0f;         // 摆动前的横向基准
  float scale = 1.0f;
  float angle = 0.0f;          // 角度（度）
  float vx = 0.0f;             // px/ms
  float vy = 0.0f;             // px/ms
  float sway_amp = 0.0f;       // 横向摆动幅度（px）
  float sway_speed = 0.0f;     // 摆动角速度（rad/ms）
  float sway_phase = 0.0f;     // 摆动初相位
  float spin = 0.0f;           // 自转速度（度/ms）
};

// 095 CAnimationDisplayer 的爆竹/新年动画：在屏幕内随机位置播放一段帧动画。
struct GameWeatherBurst {
  std::vector<wz::Node*> frames;  // 动画帧画布（按编号升序）
  SDL_FPoint pos{};               // 屏幕坐标
  size_t frame = 0;
  uint64_t frame_time = 0;        // 下一帧时间
  int frame_delay = 200;
};

// 095 TransientLayer_FireCracker / TransientLayer_NewYear 的发射器：
// 每 interval 毫秒在屏幕内随机位置生成 count 个动画。
struct GameWeatherEmitter {
  std::vector<wz::Node*> frames;
  int interval = 1000;
  int count = 1;
  int frame_delay = 200;
  uint64_t next = 0;
};

}  // namespace sdlms
