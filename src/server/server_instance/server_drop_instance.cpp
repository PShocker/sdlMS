#include "server_drop_instance.h"
#include "server_scene_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include <utility>

void server_drop_instance::handle_drop(uint64_t client_id,
                                       fbs::ClientCharacterDropT &r) {
  auto map_id = r.map_id;
  auto scene = server_scene_instance::scenes[map_id];
  ServerCharacterDropT t;
  t.client_id = client_id;
  t.payload = std::move(r.payload);
  for (const auto c : scene.clients) {
    server_response::send_to_client(c, t);
  }
}