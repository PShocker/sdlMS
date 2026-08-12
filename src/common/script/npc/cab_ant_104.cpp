#include "../script.h"

static void cab_ant_104(std::any data) {

};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"cab_ant_104"] = cab_ant_104;
  return true;
}();