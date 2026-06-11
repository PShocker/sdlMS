#include "server_drop_instance.h"
#include "server_scene_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server/server_drop.h"
#include <utility>

void server_drop_instance::save_drop(uint64_t map_id, const DropT &drop) {
  auto &scene = server_scene_instance::scenes[map_id];
  scene.drops.emplace(drop.random_id, server_drop{drop});
}

void server_drop_instance::handle_drop(uint64_t client_id,
                                       fbs::ClientCharacterDropT &r) {
  auto map_id = r.map_id;
  save_drop(map_id, *r.payload);

  const auto &scene = server_scene_instance::scenes[map_id];
  ServerCharacterDropT t;
  t.client_id = client_id;
  t.payload = std::move(r.payload);
  for (const auto c : scene.clients) {
    server_response::send_to_client(c, t);
  }
}

void server_drop_instance::handle_pick(uint64_t client_id,
                                       fbs::ClientCharacterPickT &r) {
  auto map_id = r.map_id;
  auto &scene = server_scene_instance::scenes[map_id];
  if (scene.drops.contains(r.random_id)) {
    ServerCharacterPickT t;
    t.random_id = r.random_id;
    t.client_id = client_id;
    for (const auto c : scene.clients) {
      server_response::send_to_client(c, t);
    }
    scene.drops.erase(r.random_id);
  }
}