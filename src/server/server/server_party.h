#pragma once

#include <cstdint>
#include <vector>

struct ServerParty{
  uint64_t id;
  uint64_t captain;
  std::vector<uint64_t> client_ids;
};