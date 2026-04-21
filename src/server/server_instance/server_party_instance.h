#pragma once

#include "src/server/server/server_party.h"
#include <cstdint>
#include <flat_map>


class server_party_instance {
public:
  static inline void create_party();

  static inline std::flat_map<uint64_t, server_party> partys;
};