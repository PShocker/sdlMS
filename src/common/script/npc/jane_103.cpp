#include "../script.h"

static void jane_103(std::any data) {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"jane_103"] = jane_103;
  return true;
}();