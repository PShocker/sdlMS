#include "ball_game_instance.h"
#include "src/client/game/game_ball.h"
#include <string>

void ball_game_instance::reset() { data = {}; }

void ball_game_instance::load(const fbs::CharacterBallT &bt) {
  for (const auto &b : bt.ball) {
    game_ball g_ball;
    g_ball.delay = b->delay;
    if (bt.item_id) {
      auto tmp = std::to_string(bt.item_id);
      std::u16string itm_id{tmp.begin(), tmp.end()};
      g_ball.itm_id = itm_id;
    }
    if (bt.ski_id) {
      auto tmp = std::to_string(bt.ski_id);
      std::u16string ski_id{tmp.begin(), tmp.end()};
      g_ball.ski_id = ski_id;
    }
    if (b->mob) {
      g_ball.mob_index = b->mob_index;
    }
    g_ball.pos = {b->x1, b->y1};
    g_ball.goal = {b->x2, b->y2};
    data[b->page].emplace_back(g_ball);
  }
}
