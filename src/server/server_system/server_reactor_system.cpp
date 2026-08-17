#include "server_reactor_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/common/script/script.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_reactor_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include "wz/Property.h"

void server_reactor_system::run_script(server_reactor &r) {
  if (r.revive != 0) {
    return;
  }
  auto state = r.state;
  auto next_state = server_reactor_instance::load_next_state(r.id, r.state);
  if (state == next_state && r.hit_cd <= window::dt_time) {
    r.revive = window::dt_time + 30000;
    auto script = server_reactor_instance::load_script(r.id);
    if (!script.empty()) {
      const auto &fns = script::fns();
      fns.at(script)(&r);
    }
  }
}

void server_reactor_system::run_revive(server_reactor &r) {
  if (r.revive == 0) {
    return;
  }
  if (window::dt_time >= r.revive) {
    ServerReactorT srt;
    srt.payload = std::make_unique<ReactorT>();
    srt.payload->reactor_index = r.index;
    srt.payload->state = 0;
    srt.payload->action = "revive";
    auto scenes = server_scene_instance::scenes[map_id].clients;
    for (auto c : scenes) {
      server_response::send_to_client(c, srt);
    }
    r.revive = 0;
    r.state = 0;
    r.hit_cd = 0;
    r.hit_time = 0;
  }
  return;
}

void server_reactor_system::run_hit(server_reactor &r) {
  if (r.hit_time == 0) {
    return;
  }
  auto state = r.state;
  auto next_state = server_reactor_instance::load_next_state(r.id, r.state);
  if (state == next_state) {
    return;
  }
  if (r.hit_time <= window::dt_time) {
    if (r.hit_cd <= window::dt_time) {
      r.hit_cd = server_reactor_instance::load_hit_cd(r);

      ServerReactorT srt;
      srt.payload = std::make_unique<ReactorT>();
      srt.payload->reactor_index = r.index;
      srt.payload->state = r.state;
      srt.payload->action = "hit";
      auto scenes = server_scene_instance::scenes[map_id].clients;
      for (auto c : scenes) {
        server_response::send_to_client(c, srt);
      }

      r.state = server_reactor_instance::load_next_state(r.id, r.state);
      r.hit_time = 0;
    }
  }
  return;
}

bool server_reactor_system::run() {
  auto &scenes = server_scene_instance::scenes;
  for (auto &sc : scenes | std::views::values) {
    if (sc.clients.empty()) {
      continue;
    }
    map_id = sc.map_id;
    for (auto &r : sc.reactors) {
      run_hit(r);
      run_script(r);
      run_revive(r);
    }
  }

  return true;
}