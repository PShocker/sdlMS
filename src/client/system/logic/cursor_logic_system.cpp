#include "cursor_logic_system.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_scancode.h"
#include "character_logic_system.h"
#include "npc_logic_system.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/shop_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_info_ui_system.h"
#include "src/client/system/ui/character_stat_ui_system.h"
#include "src/client/system/ui/context_menu_ui_system.h"
#include "src/client/system/ui/craft_ui_system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/keybinding_ui_system.h"
#include "src/client/system/ui/login_notice_ui_system.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/party_ui_system.h"
#include "src/client/system/ui/quest_alarm_ui_system.h"
#include "src/client/system/ui/quest_ui_system.h"
#include "src/client/system/ui/quick_slot_ui_system.h"
#include "src/client/system/ui/revive_ui_system.h"
#include "src/client/system/ui/shop_ui_system.h"
#include "src/client/system/ui/skill_ui_system.h"
#include "src/client/system/ui/statusbar_menu_ui_system.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/client/system/ui/storage_ui_system.h"
#include "src/client/system/ui/trade_ui_system.h"
#include "src/client/system/ui/worldmap_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/physic/physic.h"
#include "src/common/request/client_request.h"
#include "src/common/script/script.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>

using namespace fbs;

std::optional<GameNpc> CursorLogicSystem::cursor_npc() {
  const auto &pos = Window::mouse_pos;
  auto &camera = CameraGameInstance::camera;
  for (auto &m : NpcGameInstance::data) {
    for (auto &g_npc : m) {
      auto quest_rr = NpcGameInstance::load_quest_rect(g_npc);
      if (quest_rr.has_value()) {
        auto quest_r = quest_rr.value();
        quest_r.x -= camera.x;
        quest_r.y -= camera.y;
        if (SDL_PointInRectFloat(&pos, &quest_r)) {
          return g_npc;
        }
      }
      auto npc_rect = NpcGameInstance::load_rect(g_npc);
      npc_rect.x -= camera.x;
      npc_rect.y -= camera.y;
      if (SDL_PointInRectFloat(&pos, &npc_rect)) {
        return g_npc;
      }
    }
  }
  return std::nullopt;
}

bool CursorLogicSystem::run_package_motion() {
  auto index = PackageUiSystem::mouse_index;
  if (!index.has_value()) {
    return false;
  }
  auto active_tab = PackageUiSystem::active_tab;
  const auto &r = PackageGameInstance::data[active_tab];
  if (index.value() >= r.size()) {
    return false;
  }
  if (r[index.value()]->id.empty()) {
    return false;
  }
  return true;
}

bool CursorLogicSystem::run_vscroll_motion() {
  return CursorGameInstance::cursor_vscr;
}

void CursorLogicSystem::run_cursor_action(const std::u16string &action) {
  if (CursorGameInstance::cursor_type != action) {
    CursorGameInstance::cursor_type = action;
    CursorGameInstance::cursor_index = 0;
    CursorGameInstance::cursor_time = 0;
  }
}

bool CursorLogicSystem::run_default() {
  if (CursorGameInstance::cursor_hand.has_value()) {
    if (!CursorGameInstance::modal_overlay) {
      run_cursor_action(u"11");
      return true;
    }
  }
  if (CursorGameInstance::cursor_ui == nullptr) {
    if (!(Window::mouse_state & SDL_BUTTON_LMASK)) {
      auto npc = cursor_npc();
      if (npc.has_value()) {
        run_cursor_action(u"1");
        return true;
      }
    }
  }
  if (CursorGameInstance::cursor_ui == PackageUiSystem::render &&
      !CursorGameInstance::modal_overlay) {
    if (run_vscroll_motion()) {
      run_cursor_action(u"9");
      return true;
    }
    if (run_package_motion()) {
      run_cursor_action(u"5");
      return true;
    }
  }
  if (CursorGameInstance::cursor_ui == CharacterInfoUiSystem::render) {
    if (run_vscroll_motion()) {
      run_cursor_action(u"9");
      return true;
    }
  }
  if (CursorGameInstance::cursor_ui == SkillUiSystem::render) {
    if (run_vscroll_motion()) {
      run_cursor_action(u"9");
      return true;
    }
  }
  if (CursorGameInstance::cursor_ui == ShopUiSystem::render) {
    if (run_vscroll_motion()) {
      run_cursor_action(u"9");
      return true;
    }
  }
  if (CursorGameInstance::cursor_ui == QuestUiSystem::render) {
    if (run_vscroll_motion()) {
      run_cursor_action(u"9");
      return true;
    }
  }
  if (Window::mouse_state & SDL_BUTTON_LMASK) {
    run_cursor_action(u"12");
  } else {
    run_cursor_action(u"0");
  }

  return true;
}

void CursorLogicSystem::run_cursor_ui() {
  CursorGameInstance::cursor_ui = nullptr;
  CursorGameInstance::cursor_vscr = false;
  CursorGameInstance::modal_overlay = nullptr;
  for (auto &fn : System::render_systems | std::views::reverse) {
    if (fn == MinimapUiSystem::render) {
      if (MinimapUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == StatusbarUiSystem::render) {
      if (StatusbarUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == WorldmapUiSystem::render) {
      if (WorldmapUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == EquipUiSystem::render) {
      if (EquipUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == PackageUiSystem::render) {
      if (PackageUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == CharacterStatUiSystem::render) {
      if (CharacterStatUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == CharacterInfoUiSystem::render) {
      if (CharacterInfoUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == SkillUiSystem::render) {
      if (SkillUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == CraftUiSystem::render) {
      if (CraftUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == ReviveUiSystem::render) {
      if (ReviveUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == TradeUiSystem::render) {
      if (TradeUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == StorageUiSystem::render) {
      if (StorageUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == ContextMenuUiSystem::render) {
      if (ContextMenuUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == PartyUiSystem::render) {
      if (PartyUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == StatusbarMenuUiSystem::render) {
      if (StatusbarMenuUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == NpcDlgUiSystem::render) {
      CursorGameInstance::modal_overlay = fn;
      if (NpcDlgUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == ShopUiSystem::render) {
      CursorGameInstance::modal_overlay = fn;
      if (ShopUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == NoticeUiSystem::render) {
      CursorGameInstance::modal_overlay = fn;
      if (NoticeUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == QuestUiSystem::render) {
      if (QuestUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == QuickSlotUiSystem::render) {
      if (QuickSlotUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == QuestAlarmUiSystem::render) {
      if (QuestAlarmUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == KeybindingUiSystem::render) {
      if (KeybindingUiSystem::cursor_in()) {
        CursorGameInstance::cursor_ui = fn;
      }
    } else if (fn == LoginNoticeUiSystem::render) {
      CursorGameInstance::modal_overlay = fn;
    }
    if (CursorGameInstance::cursor_ui != nullptr) {
      break;
    }
  }
}

bool CursorLogicSystem::run_animate() {
  static auto cursor_node = WzResource::ui->find(u"Cursor.img");
  auto type_node = cursor_node->get_child(CursorGameInstance::cursor_type);
  auto child_count = type_node->children_count();
  auto index = std::to_string(CursorGameInstance::cursor_index);
  type_node = type_node->get_child(index);
  if (type_node->type == wz::Type::UOL) {
    type_node = static_cast<wz::Property<wz::WzUOL> *>(type_node)->get_uol();
  }
  int delay = 0;
  if (type_node->get_child(u"delay")) {
    delay =
        static_cast<wz::Property<int> *>(type_node->get_child(u"delay"))->get();
  }
  CursorGameInstance::cursor_time += Window::delta_time;
  if (CursorGameInstance::cursor_time >= delay) {
    CursorGameInstance::cursor_time = 0;
    CursorGameInstance::cursor_index += 1;
    CursorGameInstance::cursor_index =
        CursorGameInstance::cursor_index % child_count;
  }
  return true;
}

bool CursorLogicSystem::run() {
  run_animate();
  run_default();
  run_cursor_ui();
  return true;
}

bool CursorLogicSystem::event_cursor_hand(SDL_Event *event) {
  // 1. 事件类型检查
  if (!event || event->type != SDL_EVENT_MOUSE_BUTTON_UP ||
      event->button.button != SDL_BUTTON_LEFT) {
    return false;
  }

  auto &cursor_hand = CursorGameInstance::cursor_hand;
  auto &cursor_hand_net = CursorGameInstance::cursor_hand_net;

  // 2. 前置条件检查
  if (CursorGameInstance::cursor_ui != nullptr || !cursor_hand.has_value() ||
      cursor_hand_net.has_value()) {
    return false;
  }

  // 3. 根据光标类型处理
  switch (cursor_hand->type) {
  case CursorGameInstance::CursorHandType::kKeybind: {
    auto c = (SDL_Scancode)cursor_hand->val;
    KeyboardGameInstance::erase_input(c);
    cursor_hand = std::nullopt;
    AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
    break;
  }
  case CursorGameInstance::CursorHandType::kEquipment: {
    cursor_hand = std::nullopt;
    AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);

    break;
  }
  case CursorGameInstance::CursorHandType::kSkill: {
    cursor_hand = std::nullopt;
    AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
    break;
  }
  case CursorGameInstance::CursorHandType::kPackage: {
    int active_tab = cursor_hand->val;
    int sub_index = cursor_hand->sub_val;
    // 处理装备丢弃
    if (active_tab == static_cast<int>(ItemEnum::kEquip)) {
      auto &equip = PackageGameInstance::data[0][sub_index];
      DropT dt;

      EquipT et;
      et.equip_id = std::stoi(std::string(equip->id.begin(), equip->id.end()));
      dt.drop.Set(et);

      dt.x1 = CharacterGameInstance::self.pos.x;
      dt.y1 = CharacterGameInstance::self.pos.y;
      dt.page = CharacterGameInstance::self.page;

      ClientCharacterDropT cct;
      cct.map_id = SceneSystemInstance::map_id;
      cct.payload = std::make_unique<DropT>(dt);
      ClientRequest::send_to_host(cct);

      cursor_hand_net = {.type = CursorGameInstance::CursorHandNetType::kDrop};
    }
    // 处理物品丢弃
    else {
      auto &itm = PackageGameInstance::data[active_tab][sub_index];
      int num = ItemGameInstance::load_item_num(itm);
      auto itm_info = ItemGameInstance::load_item_info(itm->id, 0);
      bool unitPrice = itm_info->get_child(u"unitPrice");

      // 数量>1且非单价物品，弹出多选对话框
      if (num > 1 && !unitPrice) {
        NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kThrowMul;
        NoticeUiSystem::data = &itm;
        NoticeUiSystem::open();
        break; // 不执行丢弃，等待用户选择数量
      }
      bool tradeBlock = itm_info->get_child(u"tradeBlock");
      if (tradeBlock) {
        NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kTradeBlock;
        NoticeUiSystem::open();
        cursor_hand = std::nullopt;
        break; // 不执行丢弃，等待用户选择数量
      }

      // 执行物品丢弃
      DropT dt;
      ItemT it;
      it.item_id = std::stoi(std::string(itm->id.begin(), itm->id.end()));
      it.item_num = std::max(0, num);
      dt.drop.Set(it);

      dt.x1 = CharacterGameInstance::self.pos.x;
      dt.y1 = CharacterGameInstance::self.pos.y;
      dt.page = CharacterGameInstance::self.page;

      ClientCharacterDropT cct;
      cct.map_id = SceneSystemInstance::map_id;
      cct.payload = std::make_unique<DropT>(dt);
      ClientRequest::send_to_host(cct);

      cursor_hand_net = {.type = CursorGameInstance::CursorHandNetType::kDrop};
    }
    break;
  }
  default: {
    break;
  }
  }

  return false;
}

bool CursorLogicSystem::event_character(SDL_Event *event) {
  bool r = false;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT && event->button.clicks >= 2) {
      if (CursorGameInstance::cursor_ui != nullptr) {
        break;
      }
      auto others = CharacterGameInstance::others.values();
      CharacterOtherData s{.g_character = CharacterGameInstance::self};
      others.push_back(s);
      auto &camera = CameraGameInstance::camera;
      for (auto character : others) {
        auto rect = CharacterLogicSystem::load_rect(character.g_character);
        rect.x -= camera.x;
        rect.y -= camera.y;
        bool ins = SDL_PointInRectFloat(&Window::mouse_pos, &rect);
        if (ins) {
          CharacterInfoUiSystem::character = character.g_character;
          CharacterInfoUiSystem::close();
          CharacterInfoUiSystem::open();
          return true;
        }
      }
    } else if (event->button.button == SDL_BUTTON_RIGHT) {
      if (CursorGameInstance::cursor_ui != nullptr) {
        break;
      }
      auto others = CharacterGameInstance::others;
      CharacterOtherData s{.g_character = CharacterGameInstance::self};
      others[0] = s;
      auto &camera = CameraGameInstance::camera;
      for (auto [k, v] : others) {
        auto rect = CharacterLogicSystem::load_rect(v.g_character);
        rect.x -= camera.x;
        rect.y -= camera.y;
        bool ins = SDL_PointInRectFloat(&Window::mouse_pos, &rect);
        if (ins) {
          ContextMenuUiSystem::pos = Window::mouse_pos;
          ContextMenuUiSystem::client_id = k;
          ContextMenuUiSystem::client_name = v.g_character.name;
          ContextMenuUiSystem::close();
          ContextMenuUiSystem::open();
          return true;
        }
      }
    }
    break;
  }
  default: {
    break;
  }
  }
  return r;
}

bool CursorLogicSystem::event_click_npc(GameNpc npc) {
  bool r = false;
  auto npc_id = npc.id;
  auto NpcType = NpcGameInstance::load_npc_type(npc_id);
  switch (NpcType) {
  case NpcGameInstance::NpcType::kShop: {
    auto shop = ShopGameInstance::load_npc_shop(npc_id);
    ShopUiSystem::shop = shop;
    ShopUiSystem::npc = npc;
    ShopUiSystem::close();
    ShopUiSystem::open();
    r = true;
    break;
  }
  case NpcGameInstance::NpcType::kScript: {
    auto node = WzResource::npc->find(npc_id + u".img");
    auto script_node = node->find(u"info/script/0/script");
    auto script_str =
        static_cast<wz::Property<std::u16string> *>(script_node)->get();
    Script::fns().at(script_str)(nullptr);
    r = true;
    break;
  }
  case NpcGameInstance::NpcType::kQuest: {
    NpcDlgUiSystem::type = NpcDlgUiSystem::NpcDlgEnum::kQuest;
    NpcDlgUiSystem::index = 0;
    NpcDlgUiSystem::max_index = 0;
    NpcDlgUiSystem::npc_id = npc_id;
    NpcDlgUiSystem::time = Window::dt_now;
    // string.wz找d0
    NpcDlgUiSystem::text = NpcGameInstance::load_npc_text(npc_id, u"d0");
    NpcDlgUiSystem::cb = {};
    NpcDlgUiSystem::close();
    NpcDlgUiSystem::open();
    r = true;
    break;
  }
  default: {
    break;
  }
  }
  return r;
}

bool CursorLogicSystem::event_npc(SDL_Event *event) {
  if (CursorGameInstance::cursor_hand_net.has_value()) {
    return false;
  }
  bool r = false;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT && event->button.clicks >= 2) {
      if (CursorGameInstance::cursor_ui != nullptr) {
        break;
      }
      auto npc = cursor_npc();
      if (npc.has_value()) {
        r = event_click_npc(npc.value());
      }
    }
  }
  default: {
    break;
  }
  }
  return r;
}

bool CursorLogicSystem::event(SDL_Event *event) {
  if (event_cursor_hand(event)) {
    return false;
  }
  if (event_character(event)) {
    return false;
  }
  if (event_npc(event)) {
    return false;
  }

  return true;
}