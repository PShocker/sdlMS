#pragma once

#include <cstdint>
class RequestHandler {
public:
  static void HandleRequest(uint64_t client_id, void *buf, ssize_t nread);
};