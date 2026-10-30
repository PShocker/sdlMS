#include "src/server/server_instance/server_ball_instance.h"

#include "src/client/game/game_ball.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_scene_instance.h"

#include <SDL3/SDL_rect.h>

#include <cmath>

namespace sdlms {

void ServerBallInstance::HandleBall(uint64_t client_id,
                                    fbs::ClientCharacterBallT& r) {
  if (ServerClientInstance::clients_.contains(client_id)) {
    auto map_id = ServerClientInstance::clients_.at(client_id).map_id;
    auto clients = ServerSceneInstance::scenes_[map_id].clients;
    clients.erase(client_id);
    fbs::ServerCharacterBallT t;
    t.client_id = client_id;
    t.payload = std::move(r.payload);
    for (auto c : clients) {
      ServerResponse::SendToClient(c, t);
    }
  }
}

void ServerBallInstance::HandleServerB(
    const std::unique_ptr<fbs::CharacterBallT>& r) {
  const auto& src = *r->ball;

  GameBall b;
  b.delay = src.delay;
  b.mob_index = src.mob_index;
  b.start = {src.x1, src.y1};
  b.pos = b.start;
  b.goal = {src.x2, src.y2};
  b.speed = src.speed;

  b.start_offset = b.start;
  b.spawn_goal = b.goal;

  auto it = MobGameInstance::data_.find(b.mob_index);
  if (it != MobGameInstance::data_.end()) {
    const auto& mob_pos = it->second.mob.pos;
    // 095 的弹道绑定在目标 Vector2D 上：
    // 起点/终点偏移在发射瞬间固定，怪物移动时整条弹道一起平移。
    b.start_offset = {
        b.start.x - mob_pos.x,
        b.start.y - mob_pos.y,
    };
    b.spawn_goal = {
        b.goal.x + mob_pos.x,
        b.goal.y + mob_pos.y,
    };
  }

  b.flip = b.start.x < b.spawn_goal.x;

  if (b.speed > 0) {
    const float dx = b.spawn_goal.x - b.start.x;
    const float dy = b.spawn_goal.y - b.start.y;
    const float length = std::sqrt(dx * dx + dy * dy);
    b.duration =
        static_cast<uint64_t>(static_cast<double>(length) * 1000.0 / b.speed);
    if (b.duration == 0) {
      b.duration = 1;
    }
  } else {
    b.duration = 1;
  }

  b.path.assign(r->path.begin(), r->path.end());
  BallGameInstance::data_[src.page].emplace_back(std::move(b));
}

void ServerBallInstance::HandleServerBall(uint64_t client_id,
                                          fbs::ServerCharacterBallT& r) {
  HandleServerB(r.payload);
}

}  // namespace sdlms
