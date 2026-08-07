#include "../script.h"
#include "src/client/game_instance/reactor_game_instance.h"

// reactor1
static void q90008() {
  auto pos = reactor_game_instance::action_reactor->pos;
  return;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"q90008"] = q90008;
  return true;
}();