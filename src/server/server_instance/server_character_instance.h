#pragma once

#include "src/client/game/game_character.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
#include <cstdint>
using namespace fbs;

class server_character_instance {
private:
  static void save_state(uint64_t client_id, ClientCharacterLogicT &m);
  static void send_logic(uint64_t client_id, ClientCharacterLogicT &m);

public:
  static void handle_attack(uint64_t client_id, ClientCharacterAttackT &r);
  static void handle_logic(uint64_t client_id, ClientCharacterLogicT &m);
  static void handle_skill(uint64_t client_id, ClientCharacterSkillT &r);
  static void handle_chat(uint64_t client_id, ClientCharacterChatT &r);
  static void handle_server_chat(uint64_t client_id, ServerCharacterChatT &r);
  static void handle_character(uint64_t client_id, ClientCharacterT &r);

  static void handle_server_atk(uint64_t client_id, ServerCharacterAttackT &r);

  static void
  handle_ski(uint32_t ski_id, uint8_t ski_lv,
             const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
             game_character &g_character);
  static void handle_server_ski(uint64_t client_id, ServerCharacterSkillT &r);
};