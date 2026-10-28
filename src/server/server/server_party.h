#ifndef SDLMS_SRC_SERVER_SERVER_SERVER_PARTY_H_
#define SDLMS_SRC_SERVER_SERVER_SERVER_PARTY_H_

#include <cstdint>
#include <vector>
namespace sdlms {


struct ServerParty {
  uint64_t id;
  uint64_t captain;
  std::vector<uint64_t> client_ids;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_SERVER_PARTY_H_
