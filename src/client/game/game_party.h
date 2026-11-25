#pragma once

#include "src/common/flatbuffers/common.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <string>
#include <vector>

namespace sdlms {

// 组队员条目：名字 + 等级 + HP（095 PARTYDATA.party.anLevel / HP 显示）。
struct GamePartyMember {
  fbs::PlayerT player;
  uint32_t hp = 0;
  uint32_t max_hp = 0;
};

struct GameParty {
  uint64_t captain_id = 0;
  std::vector<GamePartyMember> members;  // 队长在首位（095 adwCharacterID[0] 为开队者）
};

}  // namespace sdlms