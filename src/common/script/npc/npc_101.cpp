#include "src/common/script/script.h"
namespace sdlms {


static void Npc101(std::any data) {

};

[[maybe_unused]] static const bool r = [] {
  auto& fns = Script::fns();
  fns[u"npc_101"] = Npc101;
  return true;
}();

}  // namespace sdlms
