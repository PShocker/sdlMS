#include "../script.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/window/window.h"

static void npc_18(std::any data) {
  NpcDlgUiSystem::type = NpcDlgUiSystem::NpcDlgEnum::talk;
  NpcDlgUiSystem::text = u"npc_19";
  NpcDlgUiSystem::index = 0;
  NpcDlgUiSystem::max_index = 0;
  NpcDlgUiSystem::npc_id = u"0000019";
  NpcDlgUiSystem::time = Window::dt_now;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"npc_18"] = npc_18;
  return true;
}();