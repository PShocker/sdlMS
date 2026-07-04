#include "script.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

static void state() {
  switch (npc_dlg_ui_system::index) {
  case 0: {
    auto text_node =
        wz_resource::ms->get_root()->find(u"String.img/Script/npc_8/t0");
    auto text = static_cast<wz::Property<std::u16string> *>(text_node)->get();
    npc_dlg_ui_system::text = text;
    break;
  }
  case 1: {
    auto text_node =
        wz_resource::ms->get_root()->find(u"String.img/Script/npc_8/t1");
    auto text = static_cast<wz::Property<std::u16string> *>(text_node)->get();
    npc_dlg_ui_system::text = text;
    break;
  }
  }
  npc_dlg_ui_system::time = window::dt_now;
}

static void npc_8() {
  npc_dlg_ui_system::close();
  npc_dlg_ui_system::open();
  npc_dlg_ui_system::type = npc_dlg_ui_system::npc_dlg_enum::talk;
  npc_dlg_ui_system::index = 0;
  npc_dlg_ui_system::max_index = 1;
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