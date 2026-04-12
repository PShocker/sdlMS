#pragma once

#include <cstdint>
class request_handler {
public:
  static void handle_request(uint64_t client_id, void *buf, ssize_t nread);
};