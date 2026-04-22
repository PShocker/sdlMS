#pragma once

#include "src/server/server/server_party.h"
#include <cstdint>
#include <flat_map>
class server_party_instance {
public:
  static inline void create();
  static inline std::flat_map<uint64_t, server_party> data;
};