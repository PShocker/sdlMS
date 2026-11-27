#pragma once

#include <cstdint>
#include <string>

namespace sdlms {

struct GameItemBuff {
  std::u16string id;
  uint64_t destroy = 0;
  uint64_t duration;
  // mhpR/mmpR 百分比属性按使用时的基础值折算为绝对值，记录在 buff 上，
  // 到期按相同数值扣除，避免基础值变化后算错。
  int64_t mhp_bonus = 0;
  int64_t mmp_bonus = 0;
};

}  // namespace sdlms
