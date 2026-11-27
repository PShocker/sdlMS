#pragma once

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>

namespace sdlms {

// 095 CField::RestoreClock：地图 clock 节点（x/y/width/height）定义了一个
// 显示当前时间的地图内时钟。本数据包没有 UI/UIWindow.img/Clock 数字素材，
// 这里用字体在同样的位置和尺寸内绘制 HH:MM。
class MapClockGameInstance {
 public:
  static void Load(uint32_t map_id);
  // 095 CClock 倒计时：进图时根据 info/timeLimit 计算结束时间。
  static void Enter(uint32_t map_id);
  static void Clear();
  static bool Render();
  // 屏幕顶部倒计时 HUD（Obj/etc.img/timer + clock/fontTime）。
  static bool RenderTimer();

 private:
  static inline std::optional<SDL_FRect> rect_;
  static inline uint64_t timer_end_ = 0;
};

}  // namespace sdlms
