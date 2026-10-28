#include "server_reactor_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/common/script/script.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_reactor_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include "wz/Property.h"
namespace sdlms {


void ServerReactorSystem::RunScript(ServerReactorState &r) {
  if (r.revive != 0) {
    return;
  }
  auto state = r.state;
  auto next_state = ServerReactorInstance::LoadNextState(r.id, r.state);
  if (state == next_state && r.hit_cd <= Window::dt_time_) {
    r.revive = Window::dt_time_ + 30000;
    auto Script = ServerReactorInstance::LoadScript(r.id);
    if (!Script.empty()) {
      const auto &fns = Script::fns();
      fns.at(Script)(&r);
    }
  }
}

void ServerReactorSystem::RunRevive(ServerReactorState &r) {
  if (r.revive == 0) {
    return;
  }
  if (Window::dt_time_ >= r.revive) {
    fbs::ServerReactorT srt;
    srt.payload = std::make_unique<fbs::ReactorT>();
    srt.payload->reactor_index = r.index;
    srt.payload->state = 0;
    srt.payload->action = "revive";
    auto scenes = ServerSceneInstance::scenes_[map_id_].clients;
    for (auto c : scenes) {
      ServerResponse::send_to_client(c, srt);
    }
    r.revive = 0;
    r.state = 0;
    r.hit_cd = 0;
    r.hit_time = 0;
  }
  return;
}

void ServerReactorSystem::RunHit(ServerReactorState &r) {
  if (r.hit_time == 0) {
    return;
  }
  auto state = r.state;
  auto next_state = ServerReactorInstance::LoadNextState(r.id, r.state);
  if (state == next_state) {
    return;
  }
  if (r.hit_time <= Window::dt_time_) {
    if (r.hit_cd <= Window::dt_time_) {
      r.hit_cd = ServerReactorInstance::LoadHitCd(r);

      fbs::ServerReactorT srt;
      srt.payload = std::make_unique<fbs::ReactorT>();
      srt.payload->reactor_index = r.index;
      srt.payload->state = r.state;
      srt.payload->action = "hit";
      auto scenes = ServerSceneInstance::scenes_[map_id_].clients;
      for (auto c : scenes) {
        ServerResponse::send_to_client(c, srt);
      }

      r.state = ServerReactorInstance::LoadNextState(r.id, r.state);
      r.hit_time = 0;
    }
  }
  return;
}

bool ServerReactorSystem::Run() {
  auto &scenes = ServerSceneInstance::scenes_;
  for (auto &sc : scenes | std::views::values) {
    if (sc.clients.empty()) {
      continue;
    }
    map_id_ = sc.map_id;
    for (auto &r : sc.reactors) {
      RunHit(r);
      RunScript(r);
      RunRevive(r);
    }
  }

  return true;
}

}  // namespace sdlms
