#pragma once

#include "src/common/flatbuffers/client.h"
#include <cstdint>

using namespace fbs;

class client_request {
public:
  static void heartbeat_request();

  static void scene_request(ClientSceneT client_scene);
  
  static void character_logic_request(ClientCharacterLogicT logic);
};