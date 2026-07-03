#include "script.h"

static void npc_18() {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"npc_18"] = npc_18;
  return true;
}();