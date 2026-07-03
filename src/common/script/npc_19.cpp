#include "script.h"

static void npc_19() {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"npc_19"] = npc_19;
  return true;
}();