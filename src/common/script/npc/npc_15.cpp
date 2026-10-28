#include "../script.h"

static void npc_15(std::any data) {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"npc_15"] = npc_15;
  return true;
}();