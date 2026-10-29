#pragma once

#include <vector>

namespace sdlms {

class ServerSystem {
 public:
  static inline std::vector<bool (*)()> server_systems_;
};

}  // namespace sdlms
