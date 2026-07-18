#include "server_ball_instance.h"
#include "server_client_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_ball.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"

void server_ball_instance::handle_ball(uint64_t client_id,
                                       ClientCharacterBallT &r) {
  if (server_client_instance::clients.contains(client_id)) {
    auto map_id = server_client_instance::clients.at(client_id).map_id;
    auto scenes = server_scene_instance::scenes[map_id].clients;
    ServerCharacterBallT t;
    t.client_id = client_id;
    t.payload = std::move(r.payload);
    for (auto c : scenes) {
      server_response::send_to_client(c, t);
    }
  }
}

void server_ball_instance::handle_server_ball(uint64_t client_id,
                                              ServerCharacterBallT &r) {
  game_ball g_ball;
  g_ball.delay = r.payload->ball->delay;
  g_ball.path = {r.payload->path.begin(), r.payload->path.end()};
  if (r.payload->ball->mob) {
    g_ball.mob_index = r.payload->ball->mob_index;
  }
  g_ball.pos = {r.payload->ball->x1, r.payload->ball->y1};
  g_ball.goal = {r.payload->ball->x2, r.payload->ball->y2};
  ball_game_instance::data[r.payload->ball->page].emplace_back(g_ball);
}