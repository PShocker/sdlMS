#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/script/script.h"

namespace sdlms {

static void Npc18(std::any data) {
  NpcDlgUiSystem::type_ = NpcDlgUiSystem::NpcDlgEnum::kTalk;
  NpcDlgUiSystem::text_ = u"npc_19";
  NpcDlgUiSystem::index_ = 0;
  NpcDlgUiSystem::max_index_ = 0;
  NpcDlgUiSystem::npc_id_ = u"0000019";
  NpcDlgUiSystem::time_ = Window::dt_now_;
};

[[maybe_unused]] static const bool r = [] {
  auto& fns = Script::fns();
  fns[u"npc_18"] = Npc18;
  return true;
}();

}  // namespace sdlms
