#pragma once

#include "src/common/flatbuffers/server.h"
#include <cstdint>

using namespace fbs;

class server_response {
public:
  static void heartbeat_response(uint64_t client_id);
  static void scene_response(uint64_t client_id, ServerSceneT &r);

  static void character_in_response(uint64_t client_id, ServerCharacterInT &r);
  static void character_out_response(uint64_t client_id,
                                     ServerCharacterOutT &r);
  static void character_logic_response(uint64_t client_id,
                                       ServerCharacterLogicT &r);
  static void character_attack_response(uint64_t client_id,
                                        ServerCharacterAttackT &r);
  static void character_skill_response(uint64_t client_id,
                                       ServerCharacterSkillT &r);
  static void character_chat_response(uint64_t client_id,
                                      ServerCharacterChatT &r);

  static void mob_logic_response(uint64_t client_id, ServerMobLogicT &r);
  static void mob_attack_response(uint64_t client_id, ServerMobAttackT &r);
};