#include "src/common/script/script.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <wz/Property.h>
namespace sdlms {

// roger apple

static void Q1002e(std::any data) {
  NpcDlgUiSystem::type_ = NpcDlgUiSystem::NpcDlgEnum::kQuestComplete;
  NpcDlgUiSystem::index_ = 0;
  NpcDlgUiSystem::max_index_ = 0;
  NpcDlgUiSystem::npc_id_ = u"0000003";
  NpcDlgUiSystem::time_ = Window::dt_now_;
  NpcDlgUiSystem::act_exp_ = 5;
  NpcDlgUiSystem::act_item_ = {{u"02010000", 10}};
  NpcDlgUiSystem::text_ = u"q1002e complete";
  return;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"q1002e"] = Q1002e;
  return true;
}();

}  // namespace sdlms
