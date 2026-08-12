#include "../script.h"

static void npc_18(std::any data) {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"npc_18"] = npc_18;
  return true;
}();