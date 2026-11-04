#pragma once

#include <uv.h>
#include <cstdint>

namespace sdlms {

class RequestHandler {
 public:
  // libuv 在 Windows 上提供了兼容的 ssize_t（intptr_t）。
  static void HandleRequest(uint64_t client_id, void* buf, ssize_t nread);
};

}  // namespace sdlms
