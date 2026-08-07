#include "../script.h"

static void quest_1005() {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"quest_1005"] = quest_1005;
  return true;
}();