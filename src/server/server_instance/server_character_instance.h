#pragma once

#include "src/client/game/game_character.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
#include <cstdint>
using namespace fbs;

class server_character_instance {
public:
  static void handle_attack(uint64_t client_id, ClientCharacterAttackT &r);

  static void handle_mv(uint64_t client_id, ClientCharacterMvT &m);
  static void handle_flip(uint64_t client_id, ClientCharacterFlipT &m);
  static void handle_action(uint64_t client_id, ClientCharacterActionT &m);
  static void handle_die(uint64_t client_id, ClientCharacterDieT &m);
  static void handle_fc(uint64_t client_id, ClientCharacterFcT &m);

  static void handle_server_mv(uint64_t client_id, ServerCharacterMvT &m);
  static void handle_server_flip(uint64_t client_id, ServerCharacterFlipT &m);
  static void handle_server_action(uint64_t client_id,
                                   ServerCharacterActionT &m);
  static void handle_server_die(uint64_t client_id, ServerCharacterDieT &m);
  static void handle_server_fc(uint64_t client_id, ServerCharacterFcT &m);

  static void handle_skill(uint64_t client_id, ClientCharacterSkillT &r);
  static void handle_chat(uint64_t client_id, ClientCharacterChatT &r);
  static void handle_server_chat(uint64_t client_id, ServerCharacterChatT &r);
  static void handle_character(uint64_t client_id, ClientCharacterT &r);

  static void handle_server_atk(uint64_t client_id, ServerCharacterAttackT &r);

  static game_character load_g_character(const std::unique_ptr<CharacterT> &c);
  static CharacterT load_charactert(const game_character &g);
  static void handle_server_playert(const std::unique_ptr<fbs::PlayerT> &c);

  static void
  handle_ski(uint32_t ski_id, uint8_t ski_lv,
             const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
             game_character &g_character);
  static void handle_server_ski(uint64_t client_id, ServerCharacterSkillT &r);
};