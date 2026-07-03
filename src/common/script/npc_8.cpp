#include "script.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/common/wz/wz_resource.h"

static void state() {
  switch (npc_dlg_ui_system::index) {
  case 0: {
    auto text_node = wz_resource::ms->get_root()->find(u"");
    break;
  }
  }
}

static void npc_8() {
  npc_dlg_ui_system::close();
  npc_dlg_ui_system::open();
  npc_dlg_ui_system::index = 0;
  npc_dlg_ui_system::cb = state;
  npc_dlg_ui_system::npc_id = u"0000008";
  state();
  return;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"npc_8"] = npc_8;
  return true;
}();