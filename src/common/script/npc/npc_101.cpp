#include "../script.h"

static void npc_101() {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"npc_101"] = npc_101;
  return true;
}();