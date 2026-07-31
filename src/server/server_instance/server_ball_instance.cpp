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
  game_ball b;
  b.delay = r->ball->delay;
  b.path = {
      r->path.begin(),
      r->path.end(),
  };
  if (r->ball->mob) {
    b.mob_index = r->ball->mob_index;
  }
  b.pos = {
      r->ball->x1,
      r->ball->y1,
  };
  b.goal = {
      r->ball->x2,
      r->ball->y2,
  };
  b.speed = r->ball->speed;
  if (b.mob_index.has_value()) {
    const auto &mob = mob_game_instance::data.at(b.mob_index.value());
    SDL_FPoint mob_pos{
        b.goal.x + mob.mob.pos.x,
        b.goal.x + mob.mob.pos.y,
    };
    b.flip = b.pos.x < mob_pos.x;
  } else {
    b.flip = b.pos.x < b.goal.x;
  }
  ball_game_instance::data[r->ball->page].emplace_back(b);
}

void server_ball_instance::handle_server_ball(uint64_t client_id,
                                              ServerCharacterBallT &r) {
  handle_server_b(r.payload);
}