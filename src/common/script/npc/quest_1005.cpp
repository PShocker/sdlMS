#include "../script.h"

static void Quest1005(std::any data) {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"quest_1005"] = Quest1005;
  return true;
}();