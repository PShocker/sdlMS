#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/server/server/server_scene.h"
#include <cstdint>
#include <flat_map>
#include <vector>

class server_scene_instance {
private:
  static void save_client(uint64_t client_id, fbs::ClientSceneT client_scene);
  static void send_scene_clients(uint64_t client_id, fbs::ClientSceneT client_scene);
  static void init_scene(uint64_t client_id, fbs::ClientSceneT client_scene);

public:
  static void handle_scene(uint64_t client_id, fbs::ClientSceneT client_scene);

  //   注意，地图可能存在组队地图，如果是组队图，高32位是原始地图id，低32是组队地图的组队号
  static inline std::flat_map<uint64_t, server_scene> scenes;
};