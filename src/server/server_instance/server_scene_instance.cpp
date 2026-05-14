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
#include <utility>

using namespace fbs;

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
    auto player = std::make_unique<PlayerT>(client.player_t);
    r.players.push_back(std::move(player));
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
  save_client(client_id, client_scene);
}