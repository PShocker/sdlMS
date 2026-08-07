#include "../script.h"

static void npc_14() {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"npc_14"] = npc_14;
  return true;
}();