#pragma once

#include "src/client/game/game_chatballoon.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>

namespace sdlms {

// 095 CPet：宠物跟在主人身边移动、播放动作动画，并捡取附近的掉落物。
struct GamePet {
  // 宠物道具 id（现金栏里的 8 位 id，如 u"05000000"）
  std::u16string id;
  std::u16string name;
  // 095 CPet::OnValidateStat：等级、饱食度（0~100，0 时不再捡取）、亲密度
  uint8_t level = 1;
  uint8_t fullness = 100;
  uint8_t closeness = 0;

  std::u16string action = u"stand0";
  uint32_t ani_index = 0;
  uint32_t ani_time = 0;
  // 一次性动作（jump 等）结束的时刻；0 表示循环动作
  uint64_t action_end = 0;

  SDL_FPoint pos = {0.0f, 0.0f};
  float hspeed = 0.0f;
  float vspeed = 0.0f;
  float hforce = 0.0f;
  int32_t fh = 0;
  uint8_t page = 0;
  // true 表示朝右（与 NPC/Mob 的 flip 语义一致：贴图水平翻转）
  bool flip = true;
  bool fall = false;
  // 095 HangOnBack：主人爬绳子/梯子时宠物挂在背上，不走地面物理。
  bool hang_back = false;

  // 095 CPet::DoAction -> CChatBalloon::MakeBalloon：宠物说话/回应命令的气泡。
  std::optional<GameChatballoon> chatballoon;

  // 宠物栏位，捡取请求里回传给服务器
  int8_t index = 0;
  uint64_t next_pick = 0;
  uint64_t next_idle = 0;
};

}  // namespace sdlms
