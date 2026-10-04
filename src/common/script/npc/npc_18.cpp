#include "../script.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/window/window.h"

static void npc_18(std::any data) {
  npc_dlg_ui_system::type = npc_dlg_ui_system::npc_dlg_enum::talk;
  npc_dlg_ui_system::text = u"npc_19";
  npc_dlg_ui_system::index = 0;
  npc_dlg_ui_system::max_index = 0;
  npc_dlg_ui_system::npc_id = u"0000019";
  npc_dlg_ui_system::time = window::dt_now;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"npc_18"] = npc_18;
  return true;
}();