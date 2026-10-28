#include "src/server/server_instance/server_scene_instance.h"

#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server/server_scene.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_foothold_instance.h"
#include "src/server/server_instance/server_mob_instance.h"
#include "src/server/server_instance/server_reactor_instance.h"

#include <ranges>
#include <string>
#include <utility>

namespace sdlms {

void ServerSceneInstance::CleanClient(uint64_t client_id,
                                      fbs::ClientSceneT client_scene) {
  if (ServerClientInstance::clients_.contains(client_id)) {
    // 地图切换
    auto map_id = ServerClientInstance::clients_.at(client_id).map_id;
    auto& scenes = ServerSceneInstance::scenes_[map_id];
    scenes.clients.erase(client_id);
    fbs::ServerCharacterOutT r;
    for (const auto other : scenes.clients) {
      r.client_id = client_id;
      ServerResponse::send_to_client(other, r);
    }
  }
}

void ServerSceneInstance::SaveClient(uint64_t client_id,
                                     fbs::ClientSceneT client_scene) {
  auto& clients = ServerClientInstance::clients_;
  auto& scenes = ServerSceneInstance::scenes_;

  const auto map_id = client_scene.map_id;
  const auto now = Window::dt_now_;
  auto [it, inserted] = clients.try_emplace(client_id);
  auto& client = it->second;

  if (inserted) {
    client.client_id = client_id;
    client.map_id = map_id;
    client.heartbeat = now;
    client.player_t.client_id = client_id;
    client.player_t.character = std::move(client_scene.character);
  } else {
    client.map_id = map_id;
    client.heartbeat = now;
    client.player_t.character->state = std::move(client_scene.character->state);
  }

  // 将客户端加入场景
  scenes[map_id].clients.insert(client_id);
}

void ServerSceneInstance::SendSceneClients(uint64_t client_id,
                                           fbs::ClientSceneT client_scene) {
  fbs::ServerSceneT r;
  r.map_id = client_scene.map_id;
  r.fade = true;

  auto scene = ServerSceneInstance::scenes_[client_scene.map_id];
  scene.clients.erase(client_id);
  for (const auto& c : scene.clients) {
    auto client = ServerClientInstance::clients_.at(c);
    r.players.push_back(std::make_unique<fbs::PlayerT>(client.player_t));
  }
  for (const auto& d : scene.drops | std::views::values) {
    r.drops.push_back(std::make_unique<fbs::DropT>(d.dt));
  }
  for (const auto& rs : scene.reactors) {
    auto& rt = r.reactors.emplace_back(std::make_unique<fbs::ReactorT>());
    rt->reactor_index = rs.index;
    rt->state = rs.state;
  }
  for (const auto& [k, m] : scene.mobs) {
    auto& mobt = r.mobs.emplace_back(std::make_unique<fbs::MobT>());
    mobt->mob_hp = m.hp;
    mobt->mob_index = m.index;
    mobt->mob_id = std::stoi(std::string{m.id.begin(), m.id.end()});
    mobt->state = std::make_unique<fbs::LifeStateT>();
    mobt->state->action = std::string{m.action.begin(), m.action.end()};
    mobt->state->flip = m.flip;
    mobt->state->page = m.page;
    mobt->state->x = m.pos.x;
    mobt->state->y = m.pos.y;
  }

  ServerResponse::send_to_client(client_id, r);
}

void ServerSceneInstance::SendInScene(uint64_t client_id,
                                      fbs::ClientSceneT client_scene) {
  fbs::ServerCharacterInT r;
  const auto& p = ServerClientInstance::clients_.at(client_id).player_t;

  r.player = std::make_unique<fbs::PlayerT>(p);

  auto scene = ServerSceneInstance::scenes_[client_scene.map_id];
  scene.clients.erase(client_id);
  for (const auto c : scene.clients) {
    ServerResponse::send_to_client(c, r);
  }
}

void ServerSceneInstance::SendInFade(uint64_t client_id,
                                     fbs::ClientSceneT client_scene) {
  if (client_scene.map_id == 0) {
    return;
  }
  fbs::ServerSceneT r;
  r.map_id = client_scene.map_id;
  r.fade = false;
  r.client_id = client_id;
  r.time = Window::dt_time_;
  ServerResponse::send_to_client(client_id, r);
}

void ServerSceneInstance::InitScene(uint64_t client_id,
                                    fbs::ClientSceneT client_scene) {
  //
  auto map_id = client_scene.map_id;
  if (!scenes_.contains(map_id)) {
    ServerSceneState scene;
    scene.map_id = map_id;

    ServerMobInstance::LoadMob(scene);
    ServerFootholdInstance::LoadFh(scene);
    ServerReactorInstance::LoadReactor(scene);

    scenes_[map_id] = scene;
  }
}

void ServerSceneInstance::HandleScene(uint64_t client_id,
                                      fbs::ClientSceneT client_scene) {
  if (client_scene.fade == false) {
    // 黑屏过渡
    SendInFade(client_id, client_scene);
    CleanClient(client_id, client_scene);
  } else {
    InitScene(client_id, client_scene);
    SaveClient(client_id, client_scene);
    SendSceneClients(client_id, client_scene);
    SendInScene(client_id, client_scene);
  }
}

}  // namespace sdlms
