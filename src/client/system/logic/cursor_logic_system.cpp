#include "src/client/system/logic/cursor_logic_system.h"

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
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/npc_logic_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_info_ui_system.h"
#include "src/client/system/ui/character_stat_ui_system.h"
#include "src/client/system/ui/chat_log_ui_system.h"
#include "src/client/system/ui/context_menu_ui_system.h"
#include "src/client/system/ui/craft_ui_system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/keybinding_ui_system.h"
#include "src/client/system/ui/login_notice_ui_system.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/option_ui_system.h"
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

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_scancode.h>
#include <wz/Property.h>
#include <wz/Wz.h>

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>

namespace sdlms {

std::optional<GameNpc> CursorLogicSystem::CursorNpc() {
  const auto& pos = Window::mouse_pos_;
  auto& camera = CameraGameInstance::camera_;
  for (auto& m : NpcGameInstance::data_) {
    for (auto& g_npc : m) {
      auto quest_rr = NpcGameInstance::LoadQuestRect(g_npc);
      if (quest_rr.has_value()) {
        auto quest_r = quest_rr.value();
        quest_r.x -= camera.x;
        quest_r.y -= camera.y;
        if (SDL_PointInRectFloat(&pos, &quest_r)) {
          return g_npc;
        }
      }
      auto npc_rect = NpcGameInstance::LoadRect(g_npc);
      npc_rect.x -= camera.x;
      npc_rect.y -= camera.y;
      if (SDL_PointInRectFloat(&pos, &npc_rect)) {
        return g_npc;
      }
    }
  }
  return std::nullopt;
}

bool CursorLogicSystem::RunPackageMotion() {
  auto index = PackageUiSystem::mouse_index_;
  if (!index.has_value()) {
    return false;
  }
  auto active_tab = PackageUiSystem::active_tab_;
  const auto& r = PackageGameInstance::data_[active_tab];
  if (index.value() >= r.size()) {
    return false;
  }
  if (r[index.value()]->id.empty()) {
    return false;
  }
  return true;
}

bool CursorLogicSystem::RunVscrollMotion() {
  return CursorGameInstance::cursor_vscr_;
}

void CursorLogicSystem::RunCursorAction(const std::u16string& action) {
  if (CursorGameInstance::cursor_type_ != action) {
    CursorGameInstance::cursor_type_ = action;
    CursorGameInstance::cursor_index_ = 0;
    CursorGameInstance::cursor_time_ = 0;
  }
}

bool CursorLogicSystem::RunDefault() {
  if (CursorGameInstance::cursor_hand_.has_value()) {
    if (!CursorGameInstance::modal_overlay_) {
      RunCursorAction(u"11");
      return true;
    }
  }
  // 左键按下时优先使用默认按下光标，不要让滚动条的 Cursor/7
  // (vscroll 的 u"9") 覆盖按下反馈。
  if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
    RunCursorAction(u"12");
    return true;
  }
  if (CursorGameInstance::cursor_ui_ == nullptr) {
    if (!(Window::mouse_state_ & SDL_BUTTON_LMASK)) {
      auto npc = CursorNpc();
      if (npc.has_value()) {
        RunCursorAction(u"1");
        return true;
      }
    }
  }
  if (CursorGameInstance::cursor_ui_ == PackageUiSystem::Render &&
      !CursorGameInstance::modal_overlay_) {
    if (RunVscrollMotion()) {
      RunCursorAction(u"9");
      return true;
    }
    if (RunPackageMotion()) {
      RunCursorAction(u"5");
      return true;
    }
  }
  if (CursorGameInstance::cursor_ui_ == CharacterInfoUiSystem::Render) {
    if (RunVscrollMotion()) {
      RunCursorAction(u"9");
      return true;
    }
  }
  if (CursorGameInstance::cursor_ui_ == SkillUiSystem::Render) {
    if (RunVscrollMotion()) {
      RunCursorAction(u"9");
      return true;
    }
  }
  if (CursorGameInstance::cursor_ui_ == ShopUiSystem::Render) {
    if (RunVscrollMotion()) {
      RunCursorAction(u"9");
      return true;
    }
  }
  if (CursorGameInstance::cursor_ui_ == QuestUiSystem::Render) {
    if (RunVscrollMotion()) {
      RunCursorAction(u"9");
      return true;
    }
  }
  RunCursorAction(u"0");

  return true;
}

void CursorLogicSystem::RunCursorUi() {
  CursorGameInstance::cursor_ui_ = nullptr;
  CursorGameInstance::cursor_vscr_ = false;
  CursorGameInstance::modal_overlay_ = nullptr;
  for (auto& fn : System::render_systems_ | std::views::reverse) {
    if (fn == MinimapUiSystem::Render) {
      if (MinimapUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == StatusbarUiSystem::Render) {
      if (StatusbarUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == ChatLogUiSystem::Render) {
      if (ChatLogUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == WorldmapUiSystem::Render) {
      if (WorldmapUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == EquipUiSystem::Render) {
      if (EquipUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == PackageUiSystem::Render) {
      if (PackageUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == CharacterStatUiSystem::Render) {
      if (CharacterStatUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == CharacterInfoUiSystem::Render) {
      if (CharacterInfoUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == SkillUiSystem::Render) {
      if (SkillUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == CraftUiSystem::Render) {
      if (CraftUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == ReviveUiSystem::Render) {
      CursorGameInstance::modal_overlay_ = fn;
      if (ReviveUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == TradeUiSystem::Render) {
      if (TradeUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == StorageUiSystem::Render) {
      if (StorageUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == ContextMenuUiSystem::Render) {
      if (ContextMenuUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == PartyUiSystem::Render) {
      if (PartyUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == StatusbarMenuUiSystem::Render) {
      if (StatusbarMenuUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == NpcDlgUiSystem::Render) {
      CursorGameInstance::modal_overlay_ = fn;
      if (NpcDlgUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == ShopUiSystem::Render) {
      CursorGameInstance::modal_overlay_ = fn;
      if (ShopUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == NoticeUiSystem::Render) {
      CursorGameInstance::modal_overlay_ = fn;
      if (NoticeUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == QuestUiSystem::Render) {
      if (QuestUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == QuickSlotUiSystem::Render) {
      if (QuickSlotUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == QuestAlarmUiSystem::Render) {
      if (QuestAlarmUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == KeybindingUiSystem::Render) {
      if (KeybindingUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == OptionUiSystem::Render) {
      if (OptionUiSystem::CursorIn()) {
        CursorGameInstance::cursor_ui_ = fn;
      }
    } else if (fn == LoginNoticeUiSystem::Render) {
      CursorGameInstance::modal_overlay_ = fn;
    }
    if (CursorGameInstance::cursor_ui_ != nullptr ||
        CursorGameInstance::modal_overlay_ != nullptr) {
      break;
    }
  }
}

bool CursorLogicSystem::RunAnimate() {
  static auto cursor_node = WzResource::ui_->find(u"Cursor.img");
  auto type_node = cursor_node->get_child(CursorGameInstance::cursor_type_);
  auto child_count = type_node->children_count();
  auto index = std::to_string(CursorGameInstance::cursor_index_);
  type_node = type_node->get_child(index);
  if (type_node->type == wz::Type::UOL) {
    type_node = static_cast<wz::Property<wz::WzUOL>*>(type_node)->get_uol();
  }
  int delay = 0;
  if (type_node->get_child(u"delay")) {
    delay =
        static_cast<wz::Property<int>*>(type_node->get_child(u"delay"))->get();
  }
  CursorGameInstance::cursor_time_ += Window::delta_time_;
  if (CursorGameInstance::cursor_time_ >= delay) {
    CursorGameInstance::cursor_time_ = 0;
    CursorGameInstance::cursor_index_ += 1;
    CursorGameInstance::cursor_index_ =
        CursorGameInstance::cursor_index_ % child_count;
  }
  return true;
}

bool CursorLogicSystem::Run() {
  RunAnimate();
  RunDefault();
  RunCursorUi();
  return true;
}

bool CursorLogicSystem::EventCursorHand(SDL_Event* event) {
  // 1. 事件类型检查
  if (!event || event->type != SDL_EVENT_MOUSE_BUTTON_UP ||
      event->button.button != SDL_BUTTON_LEFT) {
    return false;
  }

  auto& cursor_hand = CursorGameInstance::cursor_hand_;
  auto& cursor_hand_net = CursorGameInstance::cursor_hand_net_;

  // 2. 前置条件检查
  if (CursorGameInstance::cursor_ui_ != nullptr || !cursor_hand.has_value() ||
      cursor_hand_net.has_value()) {
    return false;
  }

  // 3. 根据光标类型处理
  switch (cursor_hand->type) {
    case CursorGameInstance::CursorHandType::kKeybind: {
      auto c = (SDL_Scancode)cursor_hand->val;
      KeyboardGameInstance::EraseInput(c);
      cursor_hand = std::nullopt;
      AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
      break;
    }
    case CursorGameInstance::CursorHandType::kEquipment: {
      cursor_hand = std::nullopt;
      AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);

      break;
    }
    case CursorGameInstance::CursorHandType::kSkill: {
      cursor_hand = std::nullopt;
      AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
      break;
    }
    case CursorGameInstance::CursorHandType::kPackage: {
      int active_tab = cursor_hand->val;
      int sub_index = cursor_hand->sub_val;
      // 处理装备丢弃
      if (active_tab == static_cast<int>(ItemEnum::kEquip)) {
        auto& equip = PackageGameInstance::data_[0][sub_index];
        fbs::DropT dt;

        fbs::EquipT et;
        et.equip_id =
            std::stoi(std::string(equip->id.begin(), equip->id.end()));
        dt.drop.Set(et);

        dt.x1 = CharacterGameInstance::self_.pos.x;
        dt.y1 = CharacterGameInstance::self_.pos.y;
        dt.page = CharacterGameInstance::self_.page;

        fbs::ClientCharacterDropT cct;
        cct.map_id = SceneSystemInstance::map_id_;
        cct.payload = std::make_unique<fbs::DropT>(dt);
        ClientRequest::SendToHost(cct);

        cursor_hand_net = {.type =
                               CursorGameInstance::CursorHandNetType::kDrop};
      }
      // 处理物品丢弃
      else {
        auto& itm = PackageGameInstance::data_[active_tab][sub_index];
        int num = ItemGameInstance::LoadItemNum(itm);
        auto itm_info = ItemGameInstance::LoadItemInfo(itm->id, 0);
        bool unit_price = itm_info->get_child(u"unitPrice");
        // 飞镖 0207 按整组处理，不能拆出单个丢弃。
        bool is_star = itm->id.starts_with(u"0207");

        // 数量>1且非单价物品，弹出多选对话框
        if (num > 1 && !unit_price && !is_star) {
          NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kThrowMul;
          NoticeUiSystem::data_ = &itm;
          NoticeUiSystem::Open();
          break;  // 不执行丢弃，等待用户选择数量
        }

        // 执行物品丢弃
        fbs::DropT dt;
        fbs::ItemT it;
        it.item_id = std::stoi(std::string(itm->id.begin(), itm->id.end()));
        it.item_num = std::max(0, num);
        dt.drop.Set(it);

        dt.x1 = CharacterGameInstance::self_.pos.x;
        dt.y1 = CharacterGameInstance::self_.pos.y;
        dt.page = CharacterGameInstance::self_.page;

        fbs::ClientCharacterDropT cct;
        cct.map_id = SceneSystemInstance::map_id_;
        cct.payload = std::make_unique<fbs::DropT>(dt);
        ClientRequest::SendToHost(cct);

        cursor_hand_net = {.type =
                               CursorGameInstance::CursorHandNetType::kDrop};
      }
      break;
    }
    default: {
      break;
    }
  }

  return false;
}

bool CursorLogicSystem::EventCharacter(SDL_Event* event) {
  bool r = false;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT &&
          event->button.clicks >= 2) {
        if (CursorGameInstance::cursor_ui_ != nullptr) {
          break;
        }
        auto others = CharacterGameInstance::others_.values();
        CharacterOtherData s{.g_character = CharacterGameInstance::self_};
        others.push_back(s);
        auto& camera = CameraGameInstance::camera_;
        for (auto character : others) {
          auto rect = CharacterLogicSystem::LoadRect(character.g_character);
          rect.x -= camera.x;
          rect.y -= camera.y;
          bool ins = SDL_PointInRectFloat(&Window::mouse_pos_, &rect);
          if (ins) {
            CharacterInfoUiSystem::character_ = character.g_character;
            CharacterInfoUiSystem::Close();
            CharacterInfoUiSystem::Open();
            return true;
          }
        }
      } else if (event->button.button == SDL_BUTTON_RIGHT) {
        if (CursorGameInstance::cursor_ui_ != nullptr) {
          break;
        }
        auto others = CharacterGameInstance::others_;
        CharacterOtherData s{.g_character = CharacterGameInstance::self_};
        others[0] = s;
        auto& camera = CameraGameInstance::camera_;
        for (auto [k, v] : others) {
          auto rect = CharacterLogicSystem::LoadRect(v.g_character);
          rect.x -= camera.x;
          rect.y -= camera.y;
          bool ins = SDL_PointInRectFloat(&Window::mouse_pos_, &rect);
          if (ins) {
            ContextMenuUiSystem::pos_ = Window::mouse_pos_;
            ContextMenuUiSystem::client_id_ = k;
            ContextMenuUiSystem::client_name_ = v.g_character.name;
            ContextMenuUiSystem::Close();
            ContextMenuUiSystem::Open();
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

bool CursorLogicSystem::EventClickNpc(GameNpc npc) {
  bool r = false;
  auto npc_id = npc.id;
  auto NpcType = NpcGameInstance::LoadNpcType(npc_id);
  switch (NpcType) {
    case NpcGameInstance::NpcType::kShop: {
      auto shop = ShopGameInstance::LoadNpcShop(npc_id);
      ShopUiSystem::shop_ = shop;
      ShopUiSystem::npc_ = npc;
      ShopUiSystem::Close();
      ShopUiSystem::Open();
      r = true;
      break;
    }
    case NpcGameInstance::NpcType::kScript: {
      auto node = WzResource::npc_->find(npc_id + u".img");
      auto script_node = node->find(u"info/script/0/script");
      auto script_str =
          static_cast<wz::Property<std::u16string>*>(script_node)->get();
      Script::fns().at(script_str)(nullptr);
      r = true;
      break;
    }
    case NpcGameInstance::NpcType::kQuest: {
      NpcDlgUiSystem::type_ = NpcDlgUiSystem::NpcDlgEnum::kQuest;
      NpcDlgUiSystem::index_ = 0;
      NpcDlgUiSystem::max_index_ = 0;
      NpcDlgUiSystem::npc_id_ = npc_id;
      NpcDlgUiSystem::time_ = Window::dt_now_;
      // string.wz找d0
      NpcDlgUiSystem::text_ = NpcGameInstance::LoadNpcText(npc_id, u"d0");
      NpcDlgUiSystem::cb_ = {};
      NpcDlgUiSystem::Close();
      NpcDlgUiSystem::Open();
      r = true;
      break;
    }
    default: {
      break;
    }
  }
  return r;
}

bool CursorLogicSystem::EventNpc(SDL_Event* event) {
  if (CursorGameInstance::cursor_hand_net_.has_value()) {
    return false;
  }
  bool r = false;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT &&
          event->button.clicks >= 2) {
        if (CursorGameInstance::cursor_ui_ != nullptr) {
          break;
        }
        auto npc = CursorNpc();
        if (npc.has_value()) {
          r = EventClickNpc(npc.value());
        }
      }
    }
    default: {
      break;
    }
  }
  return r;
}

bool CursorLogicSystem::Event(SDL_Event* event) {
  if (EventCursorHand(event)) {
    return false;
  }
  if (EventCharacter(event)) {
    return false;
  }
  if (EventNpc(event)) {
    return false;
  }

  return true;
}

}  // namespace sdlms
