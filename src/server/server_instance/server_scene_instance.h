#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/server/server/server_scene.h"
#include <cstdint>
#include <flat_map>
#include <vector>
namespace sdlms {



class ServerSceneInstance {
private:
  static void CleanClient(uint64_t client_id, fbs::ClientSceneT client_scene);
  static void SaveClient(uint64_t client_id, fbs::ClientSceneT client_scene);
  static void SendSceneClients(uint64_t client_id, fbs::ClientSceneT client_scene);
  static void SendInScene(uint64_t client_id, fbs::ClientSceneT client_scene);
  static void SendInFade(uint64_t client_id, fbs::ClientSceneT client_scene);
  static void InitScene(uint64_t client_id, fbs::ClientSceneT client_scene);

public:
  static void HandleScene(uint64_t client_id, fbs::ClientSceneT client_scene);

  //   注意，地图可能存在组队地图，如果是组队图，高32位是原始地图id，低32是组队地图的组队号
  static inline std::flat_map<uint64_t, ServerSceneState> scenes_;
};

}  // namespace sdlms
