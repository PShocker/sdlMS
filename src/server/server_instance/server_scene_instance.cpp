#include "server_scene_instance.h"
#include "SDL3/SDL_timer.h"
#include "server_client_instance.h"
#include "server_foothold_instance.h"
#include "server_mob_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server/server_scene.h"
#include <ranges>
#include <string>
#include <utility>

using namespace fbs;

void server_scene_instance::clean_client(uint64_t client_id) {
  if (server_client_instance::clients.contains(client_id)) {
    // 地图切换
    auto map_id = server_client_instance::clients.at(client_id).map_id;
    auto &scenes = server_scene_instance::scenes[map_id];
    scenes.clients.erase(client_id);
    for (const auto other : scenes.clients) {
      fbs::ServerCharacterOutT r;
      r.client_id = client_id;
      server_response::character_out_response(other, r);
    }
    server_client_instance::clients.erase(client_id);
  }
}

void server_scene_instance::save_client(uint64_t client_id,
                                        ClientSceneT client_scene) {
  auto map_id = client_scene.map_id;
  server_client client;

  client.client_id = client_id;
  client.map_id = map_id;
  client.heartbeat = window::dt_now;
  client.player_t.client_id = client_id;
  client.player_t.character = std::move(client_scene.character);

  server_client_instance::clients[client_id] = client;
  server_scene_instance::scenes[map_id].clients.insert(client_id);
}

void server_scene_instance::send_scene_clients(uint64_t client_id,
                                               ClientSceneT client_scene) {
  ServerSceneT r;
  r.map_id = client_scene.map_id;
  r.your_id = client_id;

  auto scene = server_scene_instance::scenes[client_scene.map_id];
  scene.clients.erase(client_id);
  for (const auto &c : scene.clients) {
    auto client = server_client_instance::clients.at(c);
    r.players.push_back(std::make_unique<PlayerT>(client.player_t));
  }
  for (const auto &[k, m] : scene.mobs) {
    auto &mobt = r.mobs.emplace_back(std::make_unique<MobT>());
    mobt->mob_index = m.index;
    mobt->mob_id = std::stoi(std::string{m.id.begin(), m.id.end()});
    mobt->state = std::make_unique<LifeStateT>();
    mobt->state->action = std::string{m.action.begin(), m.action.end()};
    mobt->state->flip = m.flip;
    mobt->state->page = m.page;
    mobt->state->x = m.pos.x;
    mobt->state->y = m.pos.y;
  }

  server_response::scene_response(client_id, r);
}

void server_scene_instance::send_in_scene(uint64_t client_id,
                                          ClientSceneT client_scene) {
  ServerCharacterInT r;
  PlayerT p;

  p.character = std::move(client_scene.character);
  p.client_id = client_id;
  r.player = std::make_unique<PlayerT>(std::move(p));

  auto scene = server_scene_instance::scenes[client_scene.map_id];
  scene.clients.erase(client_id);
  for (const auto c : scene.clients) {
    server_response::character_in_response(c, r);
  }
}

void server_scene_instance::init_scene(uint64_t client_id,
                                       ClientSceneT client_scene) {
  //
  auto map_id = client_scene.map_id;
  if (!scenes.contains(map_id)) {
    server_scene scene;
    scene.map_id = map_id;

    server_mob_instance::load_mob(scene);
    server_foothold_instance::load_fh(scene);

    scenes[map_id] = scene;
  }
}

void server_scene_instance::handle_scene(uint64_t client_id,
                                         ClientSceneT client_scene) {
  init_scene(client_id, client_scene);
  send_scene_clients(client_id, client_scene);
  send_in_scene(client_id, client_scene);
  clean_client(client_id);
  save_client(client_id, client_scene);
}