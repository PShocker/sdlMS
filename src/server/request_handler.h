#pragma once

#include <cstdint>

namespace sdlms {

class RequestHandler {
 public:
  static void HandleRequest(uint64_t client_id, void* buf, ssize_t nread);
};

}  // namespace sdlms
