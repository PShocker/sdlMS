#ifndef SDLMS_SRC_SERVER_REQUEST_HANDLER_H_
#define SDLMS_SRC_SERVER_REQUEST_HANDLER_H_

#include <cstdint>
namespace sdlms {

class RequestHandler {
 public:
  static void HandleRequest(uint64_t client_id, void* buf, ssize_t nread);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_REQUEST_HANDLER_H_
