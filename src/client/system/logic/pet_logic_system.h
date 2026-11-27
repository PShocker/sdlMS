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
  // 商城预览用：宠物在人物身后来回跟随（无地形物理），并播放
  // move / stand 动作。owner 是预览场景里的角色。
  static void RunPreview(GamePet& pet, const GameCharacter& owner);
};

}  // namespace sdlms
