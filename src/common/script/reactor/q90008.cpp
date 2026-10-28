#include "../script.h"
#include "src/client/game_instance/reactor_game_instance.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server/server_reactor.h"
#include "src/server/server_instance/server_drop_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include "src/server/server_system/server_reactor_system.h"

using namespace fbs;

// reactor1
static void Q90008(std::any data) {
  auto map_id = ServerReactorSystem::map_id;
  auto r = std::any_cast<ServerReactorState *>(data);
  auto pos = r->pos;
  std::vector<DropT> dts;

  DropT dt;
  dt.x1 = pos.x;
  dt.y1 = pos.y;
  dt.page = 7;
  ItemT it;
  it.item_id = 2000000;
  it.item_num = 1;
  dt.drop.Set(it);

  dts.push_back(dt);

  if (map_id == 1010) {
    it.item_id = 4031003;
    it.item_num = 1;
    dt.drop.Set(it);
    dts.push_back(dt);

    it.item_id = 4031004;
    it.item_num = 1;
    dt.drop.Set(it);
    dts.push_back(dt);
  }

  ServerReactorDropT srd;
  srd.payload = ServerDropInstance::CreateDts(dts, map_id);

  for (const auto &dt : srd.payload) {
    ServerDropInstance::SaveDrop(map_id, *dt);
  }
  auto scenes = ServerSceneInstance::scenes[map_id].clients;
  for (auto c : scenes) {
    ServerResponse::send_to_client(c, srd);
  }
  return;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"q90008"] = Q90008;
  return true;
}();