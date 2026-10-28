#include "reactor_logic_system.h"
#include "src/client/game/game_reactor.h"
#include "src/client/game_instance/reactor_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_reactor_instance.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <ranges>
#include <string>

bool ReactorLogicSystem::run_animate(GameReactor &g_reactor) {
  bool finish = false;
  if (g_reactor.hit) {
    auto node = WzResource::reactor->find(g_reactor.id + u".img");
    node = node->get_child(std::to_string(g_reactor.state));
    node = node->get_child(u"hit");
    if (node == nullptr) {
      return false;
    }
    if (node->type == wz::Type::UOL) {
      node = static_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }
    auto count = node->children_count();
    node = node->get_child(std::to_string(g_reactor.ani_index));

    if (node->type == wz::Type::UOL) {
      node = static_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }

    int32_t delay = 100;
    auto delay_node = node->get_child(u"delay");
    if (delay_node != nullptr) {
      delay = static_cast<wz::Property<int> *>(delay_node)->get();
    }
    g_reactor.ani_time += Window::delta_time;
    if (g_reactor.ani_time > delay) {
      g_reactor.ani_time = 0;
      g_reactor.ani_index += 1;
      finish = g_reactor.ani_index >= count;
      g_reactor.ani_index = g_reactor.ani_index % count;
    }
  }
  return finish;
}

void ReactorLogicSystem::run_alpha(GameReactor &g_reactor) {
  if (g_reactor.alpha < 255) {
    int alpha = g_reactor.alpha;
    alpha += Window::delta_time;
    g_reactor.alpha = std::min(255, alpha);
  }
}

bool ReactorLogicSystem::run() {
  for (auto &rs : ReactorGameInstance::data) {
    for (auto &r : rs) {
      run_alpha(r);
      if (run_animate(r)) {
        r.state = ServerReactorInstance::load_next_state(r.id, r.state);
        r.ani_index = 0;
        r.ani_time = 0;
        r.hit = false;
      }
    }
  }
  return true;
}