#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_reactor.h"
#include "src/server/server/server_scene.h"
#include <cstdint>
#include <optional>
#include <string>
class server_reactor_instance {
private:
  static uint64_t load_hit_cd(server_reactor &r);

public:
  static void load_reactor(server_scene &scene);
  static int load_next_state(const std::u16string &id, uint8_t state);
  static std::u16string load_script(const std::u16string &id);

  static void handle_s_reactor(const ReactorT &r);

  static void handle_reactor(uint64_t client_id, ClientReactorT &r);
  static void handle_server_reactor(uint64_t client_id, ServerReactorT &r);
  static void handle_server_reactor_drop(ServerReactorDropT &r);
};