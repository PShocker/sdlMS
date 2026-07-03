#include "script.h"

static void npc_15() {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"npc_15"] = npc_15;
  return true;
}();