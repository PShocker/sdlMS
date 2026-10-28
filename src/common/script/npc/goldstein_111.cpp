#include "src/common/script/script.h"
namespace sdlms {


static void Goldstein111(std::any data) {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"goldstein_111"] = Goldstein111;
  return true;
}();

}  // namespace sdlms
