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
  npc_dlg_ui_system::type = npc_dlg_ui_system::npc_dlg_enum::quest_complete;
  npc_dlg_ui_system::index = 0;
  npc_dlg_ui_system::max_index = 0;
  npc_dlg_ui_system::npc_id = u"0000003";
  npc_dlg_ui_system::time = window::dt_now;
  npc_dlg_ui_system::act_exp = 5;
  npc_dlg_ui_system::act_item = {{u"02010000", 10}};
  npc_dlg_ui_system::text = u"q1002e complete";
  return;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"q1002e"] = q1002e;
  return true;
}();