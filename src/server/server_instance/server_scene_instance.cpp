#include "server_scene_instance.h"
#include "SDL3/SDL_timer.h"
#include "server_client_instance.h"
#include "server_mob_instance.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server/server_scene.h"
#include <ranges>
#include <utility>

void server_scene_instance::save_client(uint64_t client_id,
                                        fbs::ClientSceneT client_scene) {
  auto map_id = client_scene.map_id;
  server_client s_client;

  s_client.client_id = client_id;
  s_client.map_id = map_id;
  s_client.heartbeat = SDL_GetTicks();
  s_client.fbs_player.client_id = client_id;
  s_client.fbs_player.character = std::move(client_scene.character);

  server_client_instance::clients[client_id] = s_client;
  server_scene_instance::scenes[map_id].clients[client_id] = s_client;
}

void server_scene_instance::send_scene_clients(uint64_t client_id,
                                               fbs::ClientSceneT client_scene) {
  fbs::ServerSceneT r;
  r.map_id = client_scene.map_id;
  r.your_id = client_id;

  auto scene = server_scene_instance::scenes[client_scene.map_id];
  scene.clients.erase(client_id);
  for (const auto &c : scene.clients | std::views::values) {
    auto player = std::make_unique<fbs::PlayerT>(c.fbs_player);
    r.players.push_back(std::move(player));
  }

  server_response::server_scene_response(client_id, r);
}

void server_scene_instance::send_in_scene(uint64_t client_id,
                                          fbs::ClientSceneT client_scene) {
  fbs::ServerCharacterInT r;
  fbs::PlayerT p;

  p.character = std::move(client_scene.character);
  p.client_id = client_id;
  r.player = std::make_unique<fbs::PlayerT>(std::move(p));

  auto scene = server_scene_instance::scenes[client_scene.map_id];
  scene.clients.erase(client_id);
  for (const auto &c : scene.clients | std::views::values) {
    server_response::server_character_in_response(c.client_id, r);
  }
}

void server_scene_instance::init_scene(uint64_t client_id,
                                       fbs::ClientSceneT client_scene) {
  //
  auto map_id = client_scene.map_id;
  if (!scenes.contains(map_id)) {
    server_scene s_scene;
    s_scene.map_id = map_id;
    server_mob_instance::load_mob(s_scene);
    scenes[map_id] = s_scene;
  }
}

void server_scene_instance::handle_scene(uint64_t client_id,
                                         fbs::ClientSceneT client_scene) {
  init_scene(client_id, client_scene);
  send_scene_clients(client_id, client_scene);
  send_in_scene(client_id, client_scene);
  save_client(client_id, client_scene);
}