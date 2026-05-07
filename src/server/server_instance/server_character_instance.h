#pragma once

#include "src/common/flatbuffers/client.h"
#include <cstdint>
class server_character_instance {
private:
  static void save_character_state(uint64_t client_id,
                                   fbs::ClientCharacterMoveT m);
  static void send_character_move(uint64_t client_id,
                                  fbs::ClientCharacterMoveT m);
public:
  static void handle_character_move(uint64_t client_id,
                                    fbs::ClientCharacterMoveT m);
};