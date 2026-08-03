#include "server_drop_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_scene_instance.h"

using namespace fbs;

void server_drop_system::run_fade() {
  ServerDropFadeT sdf;
  auto &scene = server_scene_instance::scenes;
  for (auto [map_id, sc] : scene) {
    sdf.drop_ids.clear();
    for (auto [random_id, dt] : sc.drops) {
      if (dt.destroy <= window::dt_now) {
        sdf.drop_ids.push_back(dt.dt.random_id);
      }
    }
    if (!sdf.drop_ids.empty()) {
      for (auto cliend_id : sc.clients) {
        server_response::send_to_client(cliend_id, sdf);
      }
    }
  }
  return;
}

void server_drop_system::run_destroy() {
  auto &scene = server_scene_instance::scenes;
  for (auto [map_id, sc] : scene) {
    std::erase_if(sc.drops, [](const auto &pair) {
      const auto &[id, drop] = pair;
      return drop.destroy <= window::dt_now;
    });
  }
}

bool server_drop_system::run() {
  run_fade();
  run_destroy();
  return true;
}