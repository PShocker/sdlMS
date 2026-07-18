#include "ball_game_instance.h"
#include "src/client/game/game_ball.h"
#include "src/common/flatbuffers/common.h"
#include <memory>
#include <string>

void ball_game_instance::reset() { data = {}; }

ClientCharacterBallT
ball_game_instance::create_ball_payload(check_mobs &cm, SDL_FPoint pos,
                                        uint64_t delay,
                                        const std::u16string &path) {
  ClientCharacterBallT ccb;
  ccb.payload = std::make_unique<CharacterBallT>();
  ccb.payload->path = {path.begin(), path.end()};
  ccb.payload->ball = std::make_unique<BallT>();
  ccb.payload->ball->delay = delay;
  ccb.payload->ball->x1 = pos.x;
  ccb.payload->ball->y1 = pos.y;
  if (!cm.data.empty()) {
    ccb.payload->ball->mob = true;
    ccb.payload->ball->mob_index = cm.data[0].mob.index;
  } else {
    ccb.payload->ball->mob = false;
  }

  return ccb;
}