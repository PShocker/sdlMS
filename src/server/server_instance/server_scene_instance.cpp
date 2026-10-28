#include "server_scene_instance.h"
#include "server_client_instance.h"
#include "server_foothold_instance.h"
#include "server_mob_instance.h"
#include "server_reactor_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server/server_scene.h"
#include <ranges>
#include <string>
#include <utility>

using namespace fbs;

void ServerSceneInstance::clean_client(uint64_t client_id,
                                         ClientSceneT client_scene) {
  if (ServerClientInstance::clients.contains(client_id)) {
    // 地图切换
    auto map_id = ServerClientInstance::clients.at(client_id).map_id;
    auto &scenes = ServerSceneInstance::scenes[map_id];
    scenes.clients.erase(client_id);
    ServerCharacterOutT r;
    for (const auto other : scenes.clients) {
      r.client_id = client_id;
      ServerResponse::send_to_client(other, r);
    }
  }
}

void ServerSceneInstance::save_client(uint64_t client_id,
                                        ClientSceneT client_scene) {
  auto &clients = ServerClientInstance::clients;
  auto &scenes = ServerSceneInstance::scenes;

  const auto map_id = client_scene.map_id;
  const auto now = Window::dt_now;
  auto [it, inserted] = clients.try_emplace(client_id);
  auto &client = it->second;

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

void ServerSceneInstance::send_scene_clients(uint64_t client_id,
                                               ClientSceneT client_scene) {
  fbs::ServerSceneT r;
  r.map_id = client_scene.map_id;
  r.fade = true;

  auto scene = ServerSceneInstance::scenes[client_scene.map_id];
  scene.clients.erase(client_id);
  for (const auto &c : scene.clients) {
    auto client = ServerClientInstance::clients.at(c);
    r.players.push_back(std::make_unique<PlayerT>(client.player_t));
  }
  for (const auto d : scene.drops | std::views::values) {
    r.drops.push_back(std::make_unique<DropT>(d.dt));
  }
  for (const auto &rs : scene.reactors) {
    auto &rt = r.reactors.emplace_back(std::make_unique<ReactorT>());
    rt->reactor_index = rs.index;
    rt->state = rs.state;
  }
  for (const auto &[k, m] : scene.mobs) {
    auto &mobt = r.mobs.emplace_back(std::make_unique<MobT>());
    mobt->mob_hp = m.hp;
    mobt->mob_index = m.index;
    mobt->mob_id = std::stoi(std::string{m.id.begin(), m.id.end()});
    mobt->state = std::make_unique<LifeStateT>();
    mobt->state->action = std::string{m.action.begin(), m.action.end()};
    mobt->state->flip = m.flip;
    mobt->state->page = m.page;
    mobt->state->x = m.pos.x;
    mobt->state->y = m.pos.y;
  }

  ServerResponse::send_to_client(client_id, r);
}

void ServerSceneInstance::send_in_scene(uint64_t client_id,
                                          ClientSceneT client_scene) {
  ServerCharacterInT r;
  const auto &p = ServerClientInstance::clients.at(client_id).player_t;

  r.player = std::make_unique<PlayerT>(p);

  auto scene = ServerSceneInstance::scenes[client_scene.map_id];
  scene.clients.erase(client_id);
  for (const auto c : scene.clients) {
    ServerResponse::send_to_client(c, r);
  }
}

void ServerSceneInstance::send_in_fade(uint64_t client_id,
                                         ClientSceneT client_scene) {
  if (client_scene.map_id == 0) {
    return;
  }
  fbs::ServerSceneT r;
  r.map_id = client_scene.map_id;
  r.fade = false;
  r.client_id = client_id;
  r.time = Window::dt_time;
  ServerResponse::send_to_client(client_id, r);
}

void ServerSceneInstance::init_scene(uint64_t client_id,
                                       ClientSceneT client_scene) {
  //
  auto map_id = client_scene.map_id;
  if (!scenes.contains(map_id)) {
    ServerSceneState scene;
    scene.map_id = map_id;

    ServerMobInstance::load_mob(scene);
    ServerFootholdInstance::load_fh(scene);
    ServerReactorInstance::load_reactor(scene);

    scenes[map_id] = scene;
  }
}

void ServerSceneInstance::handle_scene(uint64_t client_id,
                                         ClientSceneT client_scene) {
  if (client_scene.fade == false) {
    // 黑屏过渡
    send_in_fade(client_id, client_scene);
    clean_client(client_id, client_scene);
  } else {
    init_scene(client_id, client_scene);
    save_client(client_id, client_scene);
    send_scene_clients(client_id, client_scene);
    send_in_scene(client_id, client_scene);
  }
}