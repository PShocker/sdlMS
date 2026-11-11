#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sdlms {

// 怪物特效实例（v95 CMob::LoadLayer / MobSkill effect / MobBullet）
// 服务端下发 ServerMobEffect 后由 MobGameInstance::AddEffect 创建，
// 按帧速播放到最后一帧后销毁。
// move_ms > 0 时是飞行体（子弹）：从 (mx,my) 匀速移动到 (mx2,my2)，
// 帧循环播放直到到达终点；否则位置固定为怪物原点 + (ox,oy)。
struct GameMobEffect {
  uint32_t mob_index = 0;  // 所属怪物（静态特效跟随怪物移动）
  uint8_t kind = 0;        // 0 = Mob.wz，1 = Skill.wz（MobSkill）

  std::u16string path;  // wz 路径（相对对应 Files 的根）

  float ox = 0.0f;  // 相对怪物原点的偏移（静态特效）
  float oy = 0.0f;

  // 飞行体（子弹）：世界坐标的起点/终点
  float mx = 0.0f;
  float my = 0.0f;
  float mx2 = 0.0f;
  float my2 = 0.0f;
  uint64_t move_ms = 0;

  bool flip = false;
  bool done = false;
  bool loop = false;

  uint64_t start = 0;     // 开始播放时刻（绝对毫秒）
  uint32_t index = 0;     // 当前帧
  uint64_t frame_at = 0;  // 当前帧的开始时刻
  std::vector<uint32_t> delays;  // 每帧延时（懒加载）
};

}  // namespace sdlms
