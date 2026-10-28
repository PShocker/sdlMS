#include "src/server/server_system/server_drop_system.h"

#include "src/client/window/window.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_scene_instance.h"

#include <memory>

namespace sdlms {

void ServerDropSystem::RunFade() {
  fbs::ServerDropFadeT sdf;
  auto& scene = ServerSceneInstance::scenes_;
  for (auto [map_id, sc] : scene) {
    sdf.drop_ids.clear();
    for (auto [random_id, dt] : sc.drops) {
      if (dt.destroy <= Window::dt_now_) {
        sdf.drop_ids.push_back(dt.dt.random_id);
      }
    }
    if (!sdf.drop_ids.empty()) {
      for (auto cliend_id : sc.clients) {
        ServerResponse::send_to_client(cliend_id, sdf);
      }
    }
  }
  return;
}

void ServerDropSystem::RunDestroy() {
  auto& scene = ServerSceneInstance::scenes_;
  for (auto [map_id, sc] : scene) {
    std::erase_if(sc.drops, [](const auto& pair) {
      const auto& [id, drop] = pair;
      return drop.destroy <= Window::dt_now_;
    });
  }
}

void ServerDropSystem::RunDelayDrops() {
  auto& scene = ServerSceneInstance::scenes_;
  for (auto [map_id, sc] : scene) {
    auto& delay_drops = sc.delay_drops;
    for (auto [k, v] : delay_drops) {
      if (k <= Window::dt_time_) {
        fbs::ServerCreateDropT scd;
        for (auto dt : v) {
          sc.drops.emplace(dt.dt.random_id, dt);
          scd.payload.emplace_back(std::make_unique<fbs::DropT>(dt.dt));
        }
        if (scd.payload.empty()) {
          continue;
        }
        auto clients = sc.clients;
        for (auto client : clients) {
          ServerResponse::send_to_client(client, scd);
        }
      }
    }
    std::erase_if(sc.delay_drops, [](const auto& pair) {
      const auto& [delay, drop] = pair;
      return delay <= Window::dt_time_;
    });
  }
}

bool ServerDropSystem::Run() {
  RunDelayDrops();
  RunFade();
  RunDestroy();
  return true;
}

}  // namespace sdlms
