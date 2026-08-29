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

static void state() {
  switch (npc_dlg_ui_system::index) {
  case -1: {
    npc_dlg_ui_system::close();
    game_consume_item con;
    con.num = 1;
    con.id = u"02010000";
    std::polymorphic<game_item> itm{con};
    auto i = package_game_instance::add_new_item(itm);
    if (!i.has_value()) {
      notice_ui_system::open();
      notice_ui_system::type = notice_ui_system::notice_enum::shopbuy_no_space;
    } else {
      // accept quest
      game_quest q{.quest_id = u"1002.img", .index = 1};
      quest_game_instance::quests.emplace_back(q);
    }
    break;
  }
  case 1: {
    auto text_node =
        wz_resource::ms->get_root()->find(u"String.img/Script/npc_3/t0");
    auto text = static_cast<wz::Property<std::u16string> *>(text_node)->get();
    npc_dlg_ui_system::text = text;
    break;
  }
  }
  npc_dlg_ui_system::time = window::dt_now;
}

static void q1002s(std::any data) {
  if (npc_dlg_ui_system::index == 0) {
    npc_dlg_ui_system::type = npc_dlg_ui_system::npc_dlg_enum::quest;
    npc_dlg_ui_system::index = 1;
    npc_dlg_ui_system::max_index = 1;
    npc_dlg_ui_system::npc_id = u"0000003";
  }
  state();
  return;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = script::fns();
  fns[u"q1002s"] = q1002s;
  return true;
}();