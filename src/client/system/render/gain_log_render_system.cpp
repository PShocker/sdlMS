#include "gain_log_render_system.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/system/ui/quick_slot_ui_system.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <string>

void gain_log_render_system::render(game_gain_log &g_log, uint32_t i) {
  std::u16string text;
  switch (g_log.type) {
  case gain_enum::item: {
    auto itm_id = g_log.id;
    std::u16string item_name;
    if (item_game_instance::check_item(itm_id)) {
      item_name = item_game_instance::load_item_text(itm_id, u"name");
      auto item_type = item_game_instance::load_item_type(itm_id);
      auto tmp_node = wz_resource::ms->get_root()->find(u"");
      auto tmp = static_cast<wz::Property<std::u16string> *>(tmp_node)->get();
    } else {
      item_name = equip_game_instance::load_equip_name(itm_id);
      auto tmp_node = wz_resource::ms->get_root()->find(u"");
      auto tmp = static_cast<wz::Property<std::u16string> *>(tmp_node)->get();
    }
    text = u"Gain item" + item_name;
    break;
  }
  case gain_enum::experience: {
    auto exp_str = std::to_string(g_log.num);
    auto exp_str2 = std::u16string{exp_str.begin(), exp_str.end()};
    text = u"Gain exp" + exp_str2;
    break;
  }
  }
  freetype::load_size(12);
  freetype::load_bold(false);
  int x;
  int y;
  switch (quick_slot_ui_system::quickSlot) {
  case quick_slot_ui_system::quick_slot::hide: {
    break;
  }
  case quick_slot_ui_system::quick_slot::two: {
    break;
  }
  case quick_slot_ui_system::quick_slot::three: {
    break;
  }
  }
}