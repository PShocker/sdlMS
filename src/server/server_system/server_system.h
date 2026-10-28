#ifndef SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_SYSTEM_H_
#define SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_SYSTEM_H_

#include <vector>
namespace sdlms {

class ServerSystem {
public:
  static inline std::vector<bool (*)()> server_systems_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_SYSTEM_H_
