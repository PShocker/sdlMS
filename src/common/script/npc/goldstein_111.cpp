#include "../script.h"

static void goldstein_111() {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"goldstein_111"] = goldstein_111;
  return true;
}();