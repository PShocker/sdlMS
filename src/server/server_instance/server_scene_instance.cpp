#include "server_scene_instance.h"
#include "SDL3/SDL_timer.h"
#include "server_client_instance.h"
#include "server_mob_instance.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include <ranges>
#include <utility>

void server_scene_instance::save_client(uint64_t client_id,
                                        fbs::ClientSceneT client_scene) {
  auto map_id = client_scene.map_id;
  server_client s_client{
      .id = client_id,
      .map_id = map_id,
      .heartbeat = SDL_GetTicks(),
  };
  s_client.f_player.client_id = client_id;
  s_client.f_player.character = std::move(client_scene.character);
  server_client_instance::clients[client_id] = s_client;
  server_scene_instance::scenes[map_id].clients[client_id] = s_client;
}

void server_scene_instance::reply_client(uint64_t client_id,
                                         fbs::ClientSceneT client_scene) {
  fbs::ServerSceneT r;
  r.map_id = client_scene.map_id;
  r.your_id = client_id;

  auto scene = server_scene_instance::scenes[client_scene.map_id];
  scene.clients.erase(client_id);
  for (auto c : scene.clients | std::views::values) {
    auto player = std::make_unique<fbs::PlayerT>();
    r.players.push_back(std::move(player));
  }

  server_response::server_scene_response(client_id, r);
}

void server_scene_instance::init_scene(uint64_t client_id,
                                       fbs::ClientSceneT client_scene) {
  //
  if (!scenes.contains(client_scene.map_id)) {
    auto r = server_mob_instance::load_mob(client_scene.map_id);
    scenes[client_scene.map_id].mobs = r;
  }
}

void server_scene_instance::handle_scene(uint64_t client_id,
                                         fbs::ClientSceneT client_scene) {
  init_scene(client_id, client_scene);
  reply_client(client_id, client_scene);
  save_client(client_id, client_scene);
}