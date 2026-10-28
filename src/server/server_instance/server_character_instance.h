#pragma once

#include "src/client/game/game_character.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include <cstdint>
namespace sdlms {

using namespace fbs;

class ServerCharacterInstance {
private:
  static void HandleSaveState(const StateT &v, CharacterT &c);

public:
  static void HandleAttack(uint64_t client_id, ClientCharacterAttackT &r);

  static void HandleMv(uint64_t client_id, ClientCharacterMvT &m);
  static void HandleFlip(uint64_t client_id, ClientCharacterFlipT &m);
  static void HandleAction(uint64_t client_id, ClientCharacterActionT &m);
  static void HandleDie(uint64_t client_id, ClientCharacterDieT &m);
  static void HandleFc(uint64_t client_id, ClientCharacterFcT &m);

  static void HandleServerMv(uint64_t client_id, ServerCharacterMvT &m);
  static void HandleServerFlip(uint64_t client_id, ServerCharacterFlipT &m);
  static void HandleServerAction(uint64_t client_id,
                                   ServerCharacterActionT &m);
  static void HandleServerDie(uint64_t client_id, ServerCharacterDieT &m);
  static void HandleServerFc(uint64_t client_id, ServerCharacterFcT &m);

  static void HandleSkill(uint64_t client_id, ClientCharacterSkillT &r);
  static void HandleChat(uint64_t client_id, ClientCharacterChatT &r);
  static void HandleServerChat(uint64_t client_id, ServerCharacterChatT &r);
  static void HandleCharacter(uint64_t client_id, ClientCharacterT &r);

  static void HandleServerAtk(uint64_t client_id, ServerCharacterAttackT &r);

  static void LoadGCharacter(GameCharacter &g_character,
                               const std::unique_ptr<CharacterT> &c);
  static GameCharacter LoadGCharacter(const std::unique_ptr<CharacterT> &c);
  static CharacterT LoadCharactert(const GameCharacter &g);
  static void HandleServerPlayert(const std::unique_ptr<fbs::PlayerT> &c);

  static void
  HandleSki(uint32_t ski_id, uint8_t ski_lv,
             const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
             uint64_t client_id);
  static void
  HandleSki(uint32_t ski_id, uint8_t ski_lv,
             const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
             uint64_t client_id, int32_t x, int32_t y, bool flip);
  static void HandleServerSki(uint64_t client_id, ServerCharacterSkillT &r);

  static void
  HandleSState(GameCharacter &g_character,
                 const std::vector<std::unique_ptr<fbs::StateT>> &v);

  static void HandleState(uint64_t client_id, ClientCharacterStateT &r);
  static void HandleServerState(uint64_t client_id, ServerCharacterStateT &r);

  static void HandleLvUp(GameCharacter &g_character);
  static void HandleLvUp(uint64_t client_id, ClientCharacterLvUpT &r);

  static void HandleBuffItem(GameCharacter &g_character, const StateT &st);

  static void HandleMorphUse(GameCharacter &g_character, bool success);
  static void HandleScrollUse(GameCharacter &g_character, bool success);
  static void HandleItemUse(GameCharacter &g_character, const StateT &st);
};

}  // namespace sdlms
