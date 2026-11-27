#pragma once

#include "src/client/game/game_chatballoon.h"
#include "src/client/game/game_effect.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

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
  // 095 CVecCtrlPet：是否处于「追主人」状态。用滞回（80 起 / 40 停）避免
  // 在阈值附近来回切 move/stand 造成抽搐。
  bool following = false;
  // 追主人时起跳的冷却，避免主人在够不到的高台上时宠物一直原地弹跳。
  uint64_t next_jump = 0;
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

  // 宠物装备道具 id（05010000~；0=无）。主人穿戴/脱下时随宠物消息同步，
  // 远端渲染装备层与能力位判定都从这里取（不走角色帧）。
  uint32_t equip_id = 0;

  // 095 CPet::ShowEffect：地图特效（如跟丢拉回时旧位置的
  // Effect/PetEff.img/<id>/warp）。
  std::vector<GameEffect> effect;

  // 宠物栏位，捡取请求里回传给服务器
  int8_t index = 0;
  uint64_t next_pick = 0;
  uint64_t next_idle = 0;
  // 095 CPet::m_tStand / m_bRandomAction：主人静止累计时长（毫秒）与本轮
  // 5 秒窗口是否已经掷过随机动作。
  uint32_t t_stand = 0;
  bool random_action_done = false;
  // 095 CPet::Update 用 GetPos/GetPosPrev 判断主人是否移动过。
  float owner_prev_x = 0.0f;
  float owner_prev_y = 0.0f;
  bool owner_prev_valid = false;
};

}  // namespace sdlms
