#include "src/common/script/script.h"

namespace sdlms {

static void CabAnt104(std::any data) {};

[[maybe_unused]] static const bool r = [] {
  auto& fns = Script::fns();
  fns[u"cab_ant_104"] = CabAnt104;
  return true;
}();

}  // namespace sdlms
