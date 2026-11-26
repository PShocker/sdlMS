#pragma once

#include <string>

namespace sdlms {

struct GameScroll {
  std::u16string id;
  bool success;
  // 095 GW_ItemSlotEquip::GetItemGrade（option & 3）里 grade=2 的来源：
  // 诅咒卷（Item.wz spec/cursed > 0）成功后装备名字/格子变紫。
  bool cursed = false;
};

}  // namespace sdlms
