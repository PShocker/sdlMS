#include "../script.h"

static void consume_2430001(std::any data) {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"consume_2430001"] = consume_2430001;
  return true;
}();