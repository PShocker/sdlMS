#include "src/server/server_instance/server_ball_instance.h"
#include <SDL3/SDL_rect.h>
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include "src/client/game/game_ball.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
namespace sdlms {


void ServerBallInstance::HandleBall(uint64_t client_id,
                                       fbs::ClientCharacterBallT &r) {
  if (ServerClientInstance::clients_.contains(client_id)) {
    auto map_id = ServerClientInstance::clients_.at(client_id).map_id;
    auto clients = ServerSceneInstance::scenes_[map_id].clients;
    clients.erase(client_id);
    fbs::ServerCharacterBallT t;
    t.client_id = client_id;
    t.payload = std::move(r.payload);
    for (auto c : clients) {
      ServerResponse::send_to_client(c, t);
    }
  }
}

void ServerBallInstance::HandleServerB(
    const std::unique_ptr<fbs::CharacterBallT> &r) {
  const auto &src = *r->ball;

  GameBall b;
  b.delay = src.delay;
  b.mob_index = src.mob_index;
  b.pos = {src.x1, src.y1};
  b.goal = {src.x2, src.y2};
  b.speed = src.speed;
  b.flip = b.pos.x < b.goal.x;

  b.path.assign(r->path.begin(), r->path.end());

  auto it = MobGameInstance::data_.find(b.mob_index);
  if (it != MobGameInstance::data_.end()) {
    const auto &mob_pos = it->second.mob.pos;
    const float target_x = b.goal.x + mob_pos.x;
    b.flip = b.pos.x < target_x;
  }

  BallGameInstance::data_[src.page].emplace_back(std::move(b));
}

void ServerBallInstance::HandleServerBall(uint64_t client_id,
                                              fbs::ServerCharacterBallT &r) {
  HandleServerB(r.payload);
}

}  // namespace sdlms
