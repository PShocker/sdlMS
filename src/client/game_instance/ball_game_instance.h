#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
#include "src/client/game/game_character.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>

using namespace fbs;

class BallGameInstance {
private:
  static SDL_FPoint ClosestPointOnRect(const SDL_FPoint &pos,
                                          const SDL_FRect &rect);

public:
  static inline std::array<std::vector<GameBall>, 8> data_;
  static void Reset();

  static uint64_t LoadBallTime(ClientCharacterBallT &cct);
  static std::u16string LoadBallPath(const std::u16string &id);
  static std::u16string LoadCashBallPath(const std::u16string &id);
  static std::u16string LoadCashBallEffect(const std::u16string &id);

  static ClientCharacterBallT
  CreateBallPayload(CheckMobs &cm, SDL_FPoint pos, SDL_FPoint goal,
                      uint64_t delay, int page, int speed,
                      const std::u16string &path);
};