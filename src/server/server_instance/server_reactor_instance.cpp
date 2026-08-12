#include "server_reactor_instance.h"
#include "server_client_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_reactor.h"
#include "src/client/game_instance/reactor_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_drop_instance.h"
#include "wz/Property.h"
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

void server_reactor_instance::load_reactor(server_scene &scene) {
  std::vector<server_reactor> data;
  auto map_id = scene.map_id;
  auto map_node = wz_resource::load_map_node(map_id);

  auto map_reactor_node = map_node->get_child(u"reactor");
  if (map_reactor_node != nullptr) {
    int i = 0;
    for (auto [key, val] : *map_reactor_node->get_children()) {
      auto reactor_node = val[0];
      server_reactor reactor;
      reactor.id = static_cast<wz::Property<std::u16string> *>(
                       reactor_node->get_child(u"id"))
                       ->get();
      auto x = static_cast<wz::Property<int> *>(reactor_node->get_child(u"x"))
                   ->get();
      auto y = static_cast<wz::Property<int> *>(reactor_node->get_child(u"y"))
                   ->get();
      reactor.pos = SDL_FPoint{static_cast<float>(x), static_cast<float>(y)};
      reactor.index = i;
      reactor.state = 0;
      data.emplace_back(reactor);
      i++;
    }
  }
  scene.reactors = data;
}

int server_reactor_instance::load_next_state(const std::u16string &id,
                                             uint8_t state) {
  auto node = wz_resource::reactor->find(id + u".img");
  node = node->get_child(std::to_string(state));
  if (node->get_child(u"event")) {
    node = node->find(u"event/0/state");
    state = static_cast<wz::Property<int> *>(node)->get();
  }
  return state;
}

std::u16string server_reactor_instance::load_script(const std::u16string &id) {
  std::u16string r;
  auto node = wz_resource::reactor->find(id + u".img");
  if (node->get_child(u"action")) {
    node = node->get_child(u"action");
    r = static_cast<wz::Property<std::u16string> *>(node)->get();
  }
  return r;
}

uint64_t server_reactor_instance::load_hit_cd(server_reactor &r) {
  auto res = window::dt_time;
  auto node = wz_resource::reactor->find(r.id + u".img");
  node = node->get_child(std::to_string(r.state));
  if (node->get_child(u"hit")) {
    node = node->get_child(u"hit");
    for (auto [k, v] : *node->get_children()) {
      if (v[0]->get_child(u"delay")) {
        auto delay =
            static_cast<wz::Property<int> *>(v[0]->get_child(u"delay"))->get();
        res += delay;
      }
    }
  }
  return res;
}

void server_reactor_instance::handle_reactor(uint64_t client_id,
                                             ClientReactorT &r) {
  if (!server_client_instance::clients.contains(client_id)) {
    return;
  }
  const auto &atk = r.payload->attack;
  auto &scene = server_scene_instance::scenes.at(r.map_id);
  auto &reactor = scene.reactors.at(r.payload->reactor_index);
  if (reactor.hit_cd <= window::dt_time) {
    reactor.hit_cd = load_hit_cd(reactor);

    ServerReactorT srt;
    srt.payload = std::make_unique<ReactorT>();
    srt.payload->reactor_index = reactor.index;
    srt.payload->state = reactor.state;
    srt.payload->attack = std::move(r.payload->attack);
    srt.payload->action = "hit";
    auto scenes = server_scene_instance::scenes[r.map_id].clients;
    for (auto c : scenes) {
      server_response::send_to_client(c, srt);
    }

    reactor.state = load_next_state(reactor.id, reactor.state);
  }
  return;
}

void server_reactor_instance::handle_s_reactor(const ReactorT &r) {
  game_reactor *reactor = nullptr;
  for (auto &rs : reactor_game_instance::data) {
    for (auto &gr : rs) {
      if (gr.index == r.reactor_index) {
        reactor = &gr;
        break;
      }
    }
  }
  if (reactor == nullptr) {
    return;
  }
  reactor->state = r.state;
  reactor->ani_index = 0;
  reactor->ani_time = 0;
  if (r.attack) {
    reactor->hit = true;
  }
  return;
}

void server_reactor_instance::handle_server_reactor(uint64_t client_id,
                                                    ServerReactorT &r) {
  handle_s_reactor(*r.payload);
  return;
}

void server_reactor_instance::handle_server_reactor_drop(
    ServerReactorDropT &r) {
  // Drop
  for (const auto &dt : r.payload) {
    server_drop_instance::handle_server_dt(*dt);
  }
}