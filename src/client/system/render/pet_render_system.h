#pragma once

#include "src/client/game/game_pet.h"

namespace sdlms {

class PetRenderSystem {
 public:
  // 按页绘制这一页的宠物：hanging=false 画在角色之前（在主人身后），
  // hanging=true 画在角色之后（095 HangOnBack 挂在背上时盖在身体上）。
  static bool Render(uint8_t page, bool hanging);

  // 095 CPet::OnNameChanged -> CLife::MakeNameTag：宠物脚下的名字。
  // 模板 info/nameTag 有值时用 UI/NameTag.img 里对应配色的名签，否则用普通黑底。
  static void RenderNametag(const GamePet& pet);
};

}  // namespace sdlms
