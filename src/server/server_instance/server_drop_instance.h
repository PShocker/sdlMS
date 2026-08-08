#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_drop.h"
#include <cstdint>
#include <flat_map>

using namespace fbs;

class server_drop_instance {
private:
  static float cal_available_time(float y1, float y2);

public:
  static void save_drop(uint64_t map_id, const DropT &drop);

  static std::vector<std::unique_ptr<fbs::DropT>>
  create_dts(const std::vector<DropT> &dts, uint32_t map_id);

  static void handle_server_dt(const DropT &dt);
  static void handle_server_scene_dt(const DropT &dt);
  static void handle_client_drop(uint64_t client_id, ClientCharacterDropT &r);
  static void handle_pick(uint64_t client_id, ClientCharacterPickT &r);
  static void handle_server_pick(uint64_t client_id, ServerCharacterPickT &r);
  static void handle_server_drop(uint64_t client_id, ServerCharacterDropT &r);

  static void handle_server_drop_fade(ServerDropFadeT &r);
};