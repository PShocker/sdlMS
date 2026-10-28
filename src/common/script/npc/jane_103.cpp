#include "src/common/script/script.h"

namespace sdlms {

static void Jane103(std::any data) {};

[[maybe_unused]] static const bool r = [] {
  auto& fns = Script::fns();
  fns[u"jane_103"] = Jane103;
  return true;
}();

}  // namespace sdlms
