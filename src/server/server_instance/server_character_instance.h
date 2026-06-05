#pragma once

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
};