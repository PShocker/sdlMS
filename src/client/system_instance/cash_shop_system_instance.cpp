#include "cash_shop_system_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/cash_shop_ui_system.h"
#include "src/common/wz/wz_resource.h"

void CashShopSystemInstance::enter() {
  auto node = WzResource::ui->find(u"CashShopPreview.img");
  FootholdGameInstance::data = FootholdGameInstance::load(node);

  System::logic_systems = {CharacterLogicSystem::run};

  System::render_systems = {
      CashShopUiSystem::render,
  };
  System::event_systems = {
      CashShopUiSystem::event,
  };
}