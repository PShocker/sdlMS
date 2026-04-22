#pragma once

#include "src/common/flatbuffers/client.h"
#include <cstdint>
class client_request {
public:
  static void client_heartbeat_request();
  static void client_scene_request(fbs::ClientSceneT client_scene);
  static void client_visible_mob_request(fbs::ClientVisibleMobT client_visible_mob);
  static void client_character_move_request(
      fbs::ClientCharacterMoveT client_character_move);
};