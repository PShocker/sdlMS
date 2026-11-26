#pragma once

#include "src/client/game/game_chatballoon.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>

namespace sdlms {

enum class NpcState : uint8_t {
  kStand,
  kMove,
  kAction,
};

struct GameNpc {
  std::u16string id;
  int32_t rx0 = 0;
  int32_t rx1 = 0;
  int32_t fh = 0;

  SDL_FPoint pos = {0.0f, 0.0f};
  bool flip = false;

  std::u16string action;
  NpcState state = NpcState::kStand;

  uint8_t alpha = 255;

  uint32_t ani_index = 0;
  uint32_t ani_time = 0;

  // 当前状态（站立等待 / 移动 / 一次性动作）的结束时间
  uint64_t duration = 0;

  // 095：说话只由独立计时器控制，与移动/动作状态无关。
  uint64_t next_chat = 0;
  std::optional<GameChatballoon> chatballoon;
  std::optional<float> hspeed = 100.0f;
  std::optional<float> hforce;

  // 095 CNpcTemplate::bMapleTV（NPC.wz info/MapleTV）：MapleTV 摆设。
  // 模板立绘是 1x1 占位，外观由 UI/MapleTV.img 绘制——机身循环 TVon
  // 动画，广告（TVmedia/<段>/<帧>）画在 pos+(adX,adY)，段号进场随机。
  bool maple_tv = false;
  int32_t tv_ad_x = 0;      // info/MapleTVadX
  int32_t tv_ad_y = 0;      // info/MapleTVadY
  uint8_t tv_media = 0;     // 随机选中的 TVmedia 段
  uint32_t tv_ani_index = 0;  // 广告当前帧（独立于机身动画计时）
  uint32_t tv_ani_time = 0;
};

}  // namespace sdlms
