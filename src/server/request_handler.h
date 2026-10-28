#pragma once

#include <cstdint>
class RequestHandler {
public:
  static void handle_request(uint64_t client_id, void *buf, ssize_t nread);
};