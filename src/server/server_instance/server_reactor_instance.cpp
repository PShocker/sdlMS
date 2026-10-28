#include "server_reactor_instance.h"
#include "SDL3/SDL_rect.h"
#include "server_client_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_reactor.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
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

void ServerReactorInstance::LoadReactor(ServerSceneState &scene) {
  std::vector<ServerReactorState> data;
  auto map_id = scene.map_id;
  auto map_node = WzResource::LoadMapNode(map_id);

  auto map_reactor_node = map_node->get_child(u"reactor");
  if (map_reactor_node != nullptr) {
    int i = 0;
    for (auto [key, val] : *map_reactor_node->get_children()) {
      auto reactor_node = val[0];
      ServerReactorState reactor;
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

int ServerReactorInstance::LoadNextState(const std::u16string &id,
                                             uint8_t State) {
  auto node = WzResource::reactor_->find(id + u".img");
  node = node->get_child(std::to_string(State));
  if (node->get_child(u"event")) {
    node = node->find(u"event/0/state");
    State = static_cast<wz::Property<int> *>(node)->get();
  }
  return State;
}

std::u16string ServerReactorInstance::LoadScript(const std::u16string &id) {
  std::u16string r;
  auto node = WzResource::reactor_->find(id + u".img");
  if (node->get_child(u"action")) {
    node = node->get_child(u"action");
    r = static_cast<wz::Property<std::u16string> *>(node)->get();
  }
  return r;
}

uint64_t ServerReactorInstance::LoadHitCd(ServerReactorState &r) {
  auto res = Window::dt_time_;
  auto node = WzResource::reactor_->find(r.id + u".img");
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

void ServerReactorInstance::HandleReactor(uint64_t client_id,
                                             ClientReactorT &r) {
  if (!ServerClientInstance::clients_.contains(client_id)) {
    return;
  }
  auto &scene = ServerSceneInstance::scenes_.at(r.map_id);
  auto &reactor = scene.reactors.at(r.payload->reactor_index);
  if (reactor.revive != 0) {
    return;
  }
  auto delay = r.payload->delay;
  if (reactor.hit_time == 0) {
    reactor.hit_time = delay;
  } else {
    reactor.hit_time = std::min(delay, reactor.hit_time);
  }
  return;
}

void ServerReactorInstance::HandleSReactor(const ReactorT &r) {
  GameReactor *reactor = nullptr;
  for (auto &rs : ReactorGameInstance::data_) {
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
  if (r.action == "hit") {
    reactor->hit = true;
    auto tmp_state = std::to_string(r.state);
    std::u16string tmp_state2{tmp_state.begin(), tmp_state.end()};

    auto view = reactor->id |
                std::views::drop_while([](char16_t c) { return c == u'0'; });
    std::u16string r(view.begin(), view.end());

    std::u16string path = u"Reactor.img/" + r + u"/" + tmp_state2 + u"/Hit";
    AudioGameInstance::LoadAudio(path, 0);
  } else if (r.action == "revive") {
    reactor->alpha = 0;
    reactor->hit = false;
  }
  return;
}

void ServerReactorInstance::HandleServerReactor(uint64_t client_id,
                                                    fbs::ServerReactorT &r) {
  HandleSReactor(*r.payload);
  return;
}

void ServerReactorInstance::HandleServerReactorDrop(
    ServerReactorDropT &r) {
  // Drop
  for (const auto &dt : r.payload) {
    ServerDropInstance::HandleServerDt(*dt);
  }
}