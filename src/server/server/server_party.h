#pragma once

#include "src/common/flatbuffers/common.h"
#include "uv.h"
#include <cstdint>
#include <flat_set>

class server_party {
public:
  uint64_t id;
  uint64_t captain;
  std::flat_set<uint64_t> client_ids;
};