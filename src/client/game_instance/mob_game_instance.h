#pragma once

#include "src/client/game/game_mob.h"
#include "src/client/game/game_mob_effect.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

struct MobServerData {
  GameMob mob;
  std::vector<fbs::MovementT> mvs;
};

class MobGameInstance {
 public:
  static std::u16string LoadMobName(const std::u16string& id);
  static wz::Node* LoadMobInfo(const std::u16string& id);
  static wz::Node* LoadLinkMobNode(const std::u16string& id);

  static void Load(uint32_t map_id);

  // ---- 怪物特效（v95 sEffect / sHit / sBall / areaWarning / MobSkill effect）----
  // 服务端下发 ServerMobEffect 后 AddEffect，每帧 RunEffect 推进，MobRenderSystem 按怪物绘制。
  static inline std::vector<GameMobEffect> effect_;
  static void AddEffect(uint32_t mob_index, uint8_t kind,
                        const std::u16string& path, float x, float y, float x2,
                        float y2, uint64_t move_ms, bool flip, uint64_t delay);
  static void RunEffect();
  static wz::Node* LoadEffectFrameNode(const GameMobEffect& effect,
                                       uint32_t index);
  static uint32_t LoadEffectFrameDelay(wz::Node* frame);

  static inline std::flat_map<int32_t, MobServerData> data_;
};

}  // namespace sdlms
