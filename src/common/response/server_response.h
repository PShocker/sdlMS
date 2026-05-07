#pragma once

#include "src/common/flatbuffers/server.h"
#include <cstdint>
class server_response {
public:
  static void server_heartbeat_response(uint64_t client_id);
  static void server_scene_response(uint64_t client_id, fbs::ServerSceneT r);

  static void server_character_in_response(uint64_t client_id,
                                           fbs::ServerCharacterInT r);

  static void server_character_move_response(uint64_t client_id,
                                             fbs::ServerCharacterMoveT r);

  static void server_mob_move_response(uint64_t client_id,
                                       fbs::ServerMobMoveT r);
};