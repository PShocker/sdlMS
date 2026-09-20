#include "server_ball_instance.h"
#include "SDL3/SDL_rect.h"
#include "server_client_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_ball.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"

void server_ball_instance::handle_ball(uint64_t client_id,
                                       ClientCharacterBallT &r) {
  if (server_client_instance::clients.contains(client_id)) {
    auto map_id = server_client_instance::clients.at(client_id).map_id;
    auto clients = server_scene_instance::scenes[map_id].clients;
    clients.erase(client_id);
    ServerCharacterBallT t;
    t.client_id = client_id;
    t.payload = std::move(r.payload);
    for (auto c : clients) {
      server_response::send_to_client(c, t);
    }
  }
}

void server_ball_instance::handle_server_b(
    const std::unique_ptr<fbs::CharacterBallT> &r) {
  const auto &src = *r->ball;

  game_ball b;
  b.delay = src.delay;
  b.mob_index = src.mob_index;
  b.pos = {src.x1, src.y1};
  b.goal = {src.x2, src.y2};
  b.speed = src.speed;
  b.flip = b.pos.x < b.goal.x;

  b.path.assign(r->path.begin(), r->path.end());

  auto it = mob_game_instance::data.find(b.mob_index);
  if (it != mob_game_instance::data.end()) {
    const auto &mob_pos = it->second.mob.pos;
    const float target_x = b.goal.x + mob_pos.x;
    b.flip = b.pos.x < target_x;
  }

  ball_game_instance::data[src.page].emplace_back(std::move(b));
}

void server_ball_instance::handle_server_ball(uint64_t client_id,
                                              ServerCharacterBallT &r) {
  handle_server_b(r.payload);
}