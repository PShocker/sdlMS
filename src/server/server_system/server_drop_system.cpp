#include "server_drop_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_scene_instance.h"
#include <memory>

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

void server_drop_system::run_delay_drops() {
  auto &scene = server_scene_instance::scenes;
  for (auto [map_id, sc] : scene) {
    auto &delay_drops = sc.delay_drops;
    for (auto [k, v] : delay_drops) {
      if (k <= window::dt_time) {
        ServerCreateDropT scd;
        for (auto dt : v) {
          sc.drops.emplace(dt.dt.random_id, dt);
          scd.payload.emplace_back(std::make_unique<DropT>(dt.dt));
        }
        if (scd.payload.empty()) {
          continue;
        }
        auto clients = sc.clients;
        for (auto client : clients) {
          server_response::send_to_client(client, scd);
        }
      }
    }
    std::erase_if(sc.delay_drops, [](const auto &pair) {
      const auto &[delay, drop] = pair;
      return delay <= window::dt_time;
    });
  }
}

bool server_drop_system::run() {
  run_delay_drops();
  run_fade();
  run_destroy();
  return true;
}