#include "ball_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_ball.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/common/flatbuffers/common.h"
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

void ball_game_instance::reset() { data = {}; }

ClientCharacterBallT ball_game_instance::create_ball_payload(
    check_mobs &cm, SDL_FPoint pos, SDL_FPoint goal, uint64_t delay, int page,
    int speed, const std::u16string &path) {
  ClientCharacterBallT ccb;
  ccb.payload = std::make_unique<CharacterBallT>();
  ccb.payload->path = {path.begin(), path.end()};
  ccb.payload->ball = std::make_unique<BallT>();
  ccb.payload->ball->page = page;
  ccb.payload->ball->speed = speed;
  ccb.payload->ball->delay = delay;
  ccb.payload->ball->x1 = pos.x;
  ccb.payload->ball->y1 = pos.y;
  if (!cm.data.empty()) {
    ccb.payload->ball->mob = true;
    ccb.payload->ball->mob_index = cm.data[0].mob.index;
    ccb.payload->ball->x2 = 0;
    ccb.payload->ball->y2 = 0;
  } else {
    ccb.payload->ball->x2 = goal.x;
    ccb.payload->ball->y2 = goal.y;
    ccb.payload->ball->mob = false;
  }

  return ccb;
}

uint64_t ball_game_instance::load_ball_time(ClientCharacterBallT &cct) {
  auto &b = cct.payload->ball;
  if (b->mob) {
    auto p1 = SDL_FPoint{b->x1, b->y1};
    auto p2 = SDL_FPoint{b->x2, b->y2};
    auto mob_index = b->mob_index;
    auto mob_pos = mob_game_instance::data.at(mob_index).mob.pos;
    p2.x += mob_pos.x;
    p2.y += mob_pos.y;

    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float length = sqrtf(dx * dx + dy * dy);

    uint64_t dt = (length * 1000) / (float)(b->speed);
    return dt + b->delay;
  }
  return 0;
}