#include "src/common/script/script.h"
namespace sdlms {


static void Npc15(std::any data) {

};

[[maybe_unused]] static const bool r = [] {
  auto& fns = Script::fns();
  fns[u"npc_15"] = Npc15;
  return true;
}();

}  // namespace sdlms
