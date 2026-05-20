#pragma once

#include "src/common/flatbuffers/client.h"
#include <cstdint>
using namespace fbs;

class server_character_instance {
private:
  static void save_state(uint64_t client_id, ClientCharacterLogicT &m);
  static void send_logic(uint64_t client_id, CharacterLogicTypeUnion &m);

public:
  static void handle_logic(uint64_t client_id, ClientCharacterLogicT &m);
  static void handle_skill(uint64_t client_id, ClientCharacterSkillT &r);
};