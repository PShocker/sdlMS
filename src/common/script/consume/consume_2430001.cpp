#include "src/common/script/script.h"

namespace sdlms {

static void Consume2430001(std::any data) {};

[[maybe_unused]] static const bool r = [] {
  auto& fns = Script::fns();
  fns[u"consume_2430001"] = Consume2430001;
  return true;
}();

}  // namespace sdlms
