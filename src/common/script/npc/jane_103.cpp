#include "../script.h"

static void Jane103(std::any data) {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"jane_103"] = Jane103;
  return true;
}();