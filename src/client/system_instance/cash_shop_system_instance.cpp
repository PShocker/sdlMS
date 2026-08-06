#include "cash_shop_system_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/cash_shop_ui_system.h"
#include "src/common/wz/wz_resource.h"

void cash_shop_system_instance::enter() {
  auto node = wz_resource::ui->find(u"CashShopPreview.img");
  foothold_game_instance::data = foothold_game_instance::load(node);

  system::logic_systems = {character_logic_system::run};

  system::render_systems = {
      cash_shop_ui_system::render,
  };
  system::event_systems = {
      cash_shop_ui_system::event,
  };
}