#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_drop.h"
#include <cstdint>
#include <flat_map>

using namespace fbs;

class ServerDropInstance {
private:
  static float cal_available_time(float y1, float y2);

public:
  static void save_drop(uint64_t map_id, const DropT &drop);
  static void save_delay_drop(uint64_t map_id, uint64_t d, const DropT &drop);

  static std::vector<std::unique_ptr<fbs::DropT>>
  create_dts(const std::vector<DropT> &dts, uint32_t map_id);

  static void handle_server_dt(const DropT &dt);
  static void handle_server_scene_dt(const DropT &dt);
  static void handle_client_drop(uint64_t client_id, ClientCharacterDropT &r);
  static void handle_client_drop_fade(ClientDropFadeT &r);
  static void handle_pick(uint64_t client_id, ClientCharacterPickT &r);
  static void handle_server_pick(uint64_t client_id, ServerCharacterPickT &r);
  static void handle_server_drop(uint64_t client_id, ServerCharacterDropT &r);

  static void handle_server_drop_fade(ServerDropFadeT &r);

  static void handle_client_create_drop(uint64_t client_id,
                                        ClientCreateDropT &r);
  static void handle_server_create_drop(uint64_t client_id,
                                        ServerCreateDropT &r);
};