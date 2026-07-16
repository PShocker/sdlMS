#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
#include "src/client/game/game_character.h"
#include "src/common/flatbuffers/common.h"
#include <flat_map>
#include <vector>

class ball_game_instance {
public:
  static inline std::array<std::vector<game_ball>, 8> data;
  static void reset();

  static void load(const fbs::CharacterBallT &bt);
};