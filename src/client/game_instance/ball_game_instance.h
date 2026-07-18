#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
#include "src/client/game/game_character.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <flat_map>
#include <vector>

using namespace fbs;

class ball_game_instance {
public:
  static inline std::array<std::vector<game_ball>, 8> data;
  static void reset();

  static ClientCharacterBallT create_ball_payload(check_mobs &cm,
                                                  SDL_FPoint pos,
                                                  uint64_t delay,
                                                  const std::u16string& path);
};