#include "../script.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
// roger apple

static void q1002e(std::any data) {
  NpcDlgUiSystem::type = NpcDlgUiSystem::NpcDlgEnum::quest_complete;
  NpcDlgUiSystem::index = 0;
  NpcDlgUiSystem::max_index = 0;
  NpcDlgUiSystem::npc_id = u"0000003";
  NpcDlgUiSystem::time = Window::dt_now;
  NpcDlgUiSystem::act_exp = 5;
  NpcDlgUiSystem::act_item = {{u"02010000", 10}};
  NpcDlgUiSystem::text = u"q1002e complete";
  return;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"q1002e"] = q1002e;
  return true;
}();