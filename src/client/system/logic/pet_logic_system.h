#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_pet.h"

namespace sdlms {

class PetLogicSystem {
 private:
  static void RunAnimate(GamePet& pet);
  static void RunFollow(GamePet& pet, const GameCharacter& owner, bool is_self);
  static void RunPick(GamePet& pet);
  static void RunStateMachine(GamePet& pet, const GameCharacter& owner);

 public:
  static bool Run();
  // 商城预览用：只推进一只宠物的动作帧（跟随/物理由调用方处理）。
  static void AnimatePreview(GamePet& pet) { RunAnimate(pet); }
};

}  // namespace sdlms
