#ifndef SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_REGION_SKILL_INSTANCE_H_
#define SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_REGION_SKILL_INSTANCE_H_

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
namespace sdlms {


class ServerRegionSkillInstance {
public:
  static void HandleRskill(uint64_t client_id, fbs::ClientRSkillT &r);
  static void HandleSRskill(fbs::ServerRSkillT &r);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_REGION_SKILL_INSTANCE_H_
