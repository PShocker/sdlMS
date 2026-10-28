#include "../script.h"
namespace sdlms {


static void Npc14(std::any data) {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"npc_14"] = Npc14;
  return true;
}();

}  // namespace sdlms
