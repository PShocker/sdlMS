#include "../script.h"

static void consume_2430000(std::any data) {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"consume_2430000"] = consume_2430000;
  return true;
}();