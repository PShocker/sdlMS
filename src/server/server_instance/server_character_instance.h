#ifndef SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_CHARACTER_INSTANCE_H_
#define SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_CHARACTER_INSTANCE_H_

#include "src/client/game/game_character.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include <cstdint>
namespace sdlms {


class ServerCharacterInstance {
private:
  static void HandleSaveState(const fbs::StateT &v, fbs::CharacterT &c);

public:
  static void HandleAttack(uint64_t client_id, fbs::ClientCharacterAttackT &r);

  static void HandleMv(uint64_t client_id, fbs::ClientCharacterMvT &m);
  static void HandleFlip(uint64_t client_id, fbs::ClientCharacterFlipT &m);
  static void HandleAction(uint64_t client_id, fbs::ClientCharacterActionT &m);
  static void HandleDie(uint64_t client_id, fbs::ClientCharacterDieT &m);
  static void HandleFc(uint64_t client_id, fbs::ClientCharacterFcT &m);

  static void HandleServerMv(uint64_t client_id, fbs::ServerCharacterMvT &m);
  static void HandleServerFlip(uint64_t client_id, fbs::ServerCharacterFlipT &m);
  static void HandleServerAction(uint64_t client_id,
                                   fbs::ServerCharacterActionT &m);
  static void HandleServerDie(uint64_t client_id, fbs::ServerCharacterDieT &m);
  static void HandleServerFc(uint64_t client_id, fbs::ServerCharacterFcT &m);

  static void HandleSkill(uint64_t client_id, fbs::ClientCharacterSkillT &r);
  static void HandleChat(uint64_t client_id, fbs::ClientCharacterChatT &r);
  static void HandleServerChat(uint64_t client_id, fbs::ServerCharacterChatT &r);
  static void HandleCharacter(uint64_t client_id, fbs::ClientCharacterT &r);

  static void HandleServerAtk(uint64_t client_id, fbs::ServerCharacterAttackT &r);

  static void LoadGCharacter(GameCharacter &g_character,
                               const std::unique_ptr<fbs::CharacterT> &c);
  static GameCharacter LoadGCharacter(const std::unique_ptr<fbs::CharacterT> &c);
  static fbs::CharacterT LoadCharactert(const GameCharacter &g);
  static void HandleServerPlayert(const std::unique_ptr<fbs::PlayerT> &c);

  static void
  HandleSki(uint32_t ski_id, uint8_t ski_lv,
             const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
             uint64_t client_id);
  static void
  HandleSki(uint32_t ski_id, uint8_t ski_lv,
             const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
             uint64_t client_id, int32_t x, int32_t y, bool flip);
  static void HandleServerSki(uint64_t client_id, fbs::ServerCharacterSkillT &r);

  static void
  HandleSState(GameCharacter &g_character,
                 const std::vector<std::unique_ptr<fbs::StateT>> &v);

  static void HandleState(uint64_t client_id, fbs::ClientCharacterStateT &r);
  static void HandleServerState(uint64_t client_id, fbs::ServerCharacterStateT &r);

  static void HandleLvUp(GameCharacter &g_character);
  static void HandleLvUp(uint64_t client_id, fbs::ClientCharacterLvUpT &r);

  static void HandleBuffItem(GameCharacter &g_character, const fbs::StateT &st);

  static void HandleMorphUse(GameCharacter &g_character, bool success);
  static void HandleScrollUse(GameCharacter &g_character, bool success);
  static void HandleItemUse(GameCharacter &g_character, const fbs::StateT &st);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_CHARACTER_INSTANCE_H_
