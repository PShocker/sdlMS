#pragma once

#include <cstdint>
#include <vector>

class server_party {
public:
  uint64_t id;
  uint64_t captain;
  std::vector<uint64_t> client_ids;
};