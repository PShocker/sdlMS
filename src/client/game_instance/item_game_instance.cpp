#include "src/client/game_instance/item_game_instance.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/pet_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/game_instance/weather_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/ui/minigame_card_ui_system.h"
#include "src/client/system/ui/minigame_omok_ui_system.h"
#include "src/client/system/ui/minigame_setup_ui_system.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/player_shop_ui_system.h"
#include "src/client/system/ui/weather_dlg_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"

#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <flat_map>
#include <ranges>
#include <string>


namespace sdlms {

namespace {

// 小游戏桌（0408）系列：Item.wz/Etc/0408.img 里的安装类桌具
//（五子棋 04080000 / 卡片配对 04080100，名称在 String.wz/Etc.img/40800xx）。
bool IsMinigameTable(const std::u16string& id) {
  return id.starts_with(u"0408");
}

// 卡片配对桌（04080100~）。
bool IsMatchCardsTable(const std::u16string& id) {
  return id.starts_with(u"040801");
}

// 骑宠（095 TamingMob）：0190/0191/0193 系列坐骑（猪猪 Hog 等）。
// 名称/图标不在 WZ 里（精简客户端无 TamingMob 图像），代码合成。
bool IsMountItem(const std::u16string& id) {
  return id.starts_with(u"0190") || id.starts_with(u"0191") ||
         id.starts_with(u"0193");
}

// 骑宠形态 id（095：道具 → 载具形态，本项目用 TamingMob_000.wz 里实际存在的形态）。
// 01902000/01902001/01902002…：形态取道具后四位。
std::u16string LoadMountForm(const std::u16string& item_id) {
  if (item_id.size() < 8) {
    return u"1000";
  }
  // 01902000 → 1000；其余形态（0001~0025）由道具尾号指定。
  auto form = item_id.substr(4);
  return form;
}

}  // namespace

bool ItemGameInstance::CheckItem(const std::u16string& id) {
  auto r = id.substr(0, 2);
  if (r == u"01") {
    return false;
  }
  return true;
}

std::u16string ItemGameInstance::LoadItemText(const std::u16string& id,
                                               const std::u16string& val) {
  // 骑宠名称代码内置（095：Hog/猪猪等，本项目 String.wz 无此条目）。
  if (IsMountItem(id)) {
    if (val == u"name") {
      return id.starts_with(u"0190") ? u"Hog \u9a91\u5ba0"
             : id.starts_with(u"0193") ? u"Riding Mount"
                                       : u"Taming Mob";
    }
    return u"\u4f7f\u7528\u540e\u9a91\u4e58\uff1b\u518d\u6b21\u4f7f\u7528\u4e0b\u9a91\u3002";
  }
  auto item_type = LoadItemType(id);
  // 小游戏桌（0408）的使用类型是 Ins，但名称在 String.wz/Etc.img 下。
  auto str_type = IsMinigameTable(id) ? u"Etc" : item_type;
  auto str_node = WzResource::string_->find(str_type + u".img");
  if (str_type == u"Etc") {
    str_node = str_node->get_child("Etc");
  }
  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });
  std::u16string result(view.begin(), view.end());
  str_node = str_node->get_child(result)->get_child(val);
  return TextGameInstance::LoadRstr(str_node);
}

std::u16string ItemGameInstance::LoadItemType(const std::u16string& id) {
  auto str = id;
  if (str.length() < 8) {
    str.insert(0, 8 - str.size(), u'0');
  }
  if (str == u"00000000") {
    // meso
    return u"Etc";
  }
  static const std::flat_map<std::u16string, std::u16string> types = {
      {u"05", u"Cash"}, {u"02", u"Consume"}, {u"04", u"Etc"},
      {u"03", u"Ins"},  {u"50", u"Pet"},     {u"09", u"Special"},
  };
  auto r = str.substr(0, 2);
  if (IsMinigameTable(str)) {
    // 095：小游戏桌按安装类（Setup）处理（使用=开房），但数据放在 Item.wz/Etc。
    return u"Ins";
  }
  if (IsMountItem(str)) {
    // 095：坐骑道具（190xxxxx：Hog/猪猪等），使用=上骑/下骑。
    return u"TamingMob";
  }
  if (auto it = types.find(r); it != types.end()) {
    // 宠物道具（05000000~）在现金栏里，但数据放在 Item.wz/Pet/<7 位 id>.img，
    // 名字在 String.wz/Pet.img 下，这里单独区分，避免按 Cash 找图/文字时崩。
    if (it->second == u"Cash" && str.starts_with(u"0500")) {
      return u"Pet";
    }
    return it->second;
  }
  // 旧存档可能存在无效/未知物品 ID，返回 Etc 避免读档直接抛异常。
  return u"Etc";
}

wz::Node* ItemGameInstance::LoadItemInfo(const std::u16string& id,
                                          uint32_t num) {
  wz::Node* node = nullptr;
  if (IsMountItem(id)) {
    // 骑宠（095 TamingMob）：精简客户端无坐骑图像，合成 info 节点
    //（icon 用现成画布），保证背包/Tooltip 渲染不踩空。
    static wz::Node* mount_info = [] {
      auto* n = new wz::Node(wz::Type::SubProperty, nullptr);
      auto* icon =
          WzResource::ui_->find(u"Minigame.img/Popup/icon_public");
      n->appendChild(u"icon", icon);
      n->appendChild(u"iconRaw", icon);
      return n;
    }();
    return mount_info;
  }
  if (id == u"00000000") {
    // 金币图标随数量变化：1-50 / 51-100 / 101-1000 / 1001+。
    std::u16string meso_id = u"09000000";
    if (num > 1000) {
      meso_id = u"09000003";
    } else if (num > 100) {
      meso_id = u"09000002";
    } else if (num > 50) {
      meso_id = u"09000001";
    }
    node = WzResource::item_->find(u"Special/0900.img/" + meso_id +
                                   u"/iconRaw");
    return node;
  }
  auto type = LoadItemType(id);
  if (type == u"Ins") {
    type = u"Install";
  }
  // 小游戏桌：使用类型是 Install，但 item 数据放在 Item.wz/Etc 下。
  if (IsMinigameTable(id)) {
    type = u"Etc";
  }
  if (type == u"Consume" || type == u"Etc" || type == u"Install") {
    auto r = id.substr(0, 4) + u".img";
    if (auto image = WzResource::item_->find(type + u"/" + r);
        image != nullptr) {
      if (auto item = image->get_child(id); item != nullptr) {
        node = item->get_child(u"info");
      }
    }
  } else if (type == u"Cash") {
    auto r = id.substr(0, 4) + u".img";
    if (auto image = WzResource::item_->find(u"Cash/" + r); image != nullptr) {
      if (auto item = image->get_child(id); item != nullptr) {
        node = item->get_child(u"info");
      }
    }
  } else if (type == u"Pet") {
    // 095：宠物道具的影子数据在 Item.wz/Pet/<7 位 id>.img，info 里带
    // icon/iconRaw/life/pickupItem 等字段。
    auto view = id | std::views::drop_while([](char16_t c) {
      return c == u'0';
    });
    std::u16string key(view.begin(), view.end());
    if (key.size() < 7) {
      key.insert(0, 7 - key.size(), u'0');
    }
    if (auto image = WzResource::item_->find(u"Pet/" + key + u".img");
        image != nullptr) {
      node = image->get_child(u"info");
    }
  }
  return node;
}

int ItemGameInstance::LoadItemFloatType(const std::u16string& id) {
  // 095 CField::BlowWeather：天气现金道具的 info/floatType 决定 FloatNotice 底板。
  auto info = LoadItemInfo(id, 0);
  if (info == nullptr) {
    return 0;
  }
  auto* node = info->get_child(u"floatType");
  if (node == nullptr) {
    return 0;
  }
  return static_cast<wz::Property<int32_t>*>(node)->get();
}

bool ItemGameInstance::LoadItemWeather(const std::u16string& id,
                                       ItemWeatherInfo& out) {
  // 095 get_cashslot_item_type：id/10000 == 512 的现金道具是天气类（type 16），
  // 该批道具不一定带 floatType（05120000~05120002），底板序号缺省为 0。
  if (!id.starts_with(u"0512")) {
    return false;
  }
  auto info = LoadItemInfo(id, 0);
  if (info == nullptr) {
    return false;
  }
  out = ItemWeatherInfo{};
  auto load_int = [info](const char16_t* name) {
    auto* node = info->get_child(name);
    if (node == nullptr || node->type != wz::Type::Int) {
      return 0;
    }
    return static_cast<wz::Property<int32_t>*>(node)->get();
  };
  out.float_type = load_int(u"floatType");
  out.type = load_int(u"type");
  out.direction = load_int(u"direction");
  out.speed = load_int(u"speed");
  out.is_bgm = load_int(u"isBgmOrEffect");
  out.repeat = load_int(u"repeat");
  if (auto* path = info->get_child(u"path");
      path != nullptr && path->type == wz::Type::String) {
    out.path = static_cast<wz::Property<std::u16string>*>(path)->get();
  }
  if (auto* bgm = info->get_child(u"bgm");
      bgm != nullptr && bgm->type == wz::Type::String) {
    out.bgm = static_cast<wz::Property<std::u16string>*>(bgm)->get();
  }
  return true;
}

std::polymorphic<GameItem> ItemGameInstance::LoadItem(const std::u16string& id,
                                                      uint32_t num) {
  auto item_type = ItemGameInstance::LoadItemType(id);
  if (item_type == u"Cash") {
    GameCashItem cash;
    cash.id = id;
    return std::polymorphic<GameItem>(cash);
  } else if (item_type == u"Consume") {
    GameConsumeItem consume;
    consume.id = id;
    consume.num = num;
    return std::polymorphic<GameItem>(consume);
  } else if (item_type == u"Etc") {
    GameEtcItem etc;
    etc.id = id;
    etc.num = num;
    return std::polymorphic<GameItem>(std::in_place_type<GameEtcItem>, etc);
  } else if (item_type == u"Ins") {
    GameInstallItem install;
    install.id = id;
    return std::polymorphic<GameItem>(install);
  } else if (item_type == u"TamingMob") {
    // 骑宠道具进安装栏（与椅子同栏，双击上/下骑）。
    GameInstallItem install;
    install.id = id;
    return std::polymorphic<GameItem>(install);
  } else if (item_type == u"Pet" || item_type == u"Special") {
    GameCashItem cash;
    cash.id = id;
    return std::polymorphic<GameItem>(cash);
  }
  // 未知类型回退为 Etc，避免存档中残留数据生成空 polymorphic 对象。
  GameEtcItem etc;
  etc.id = id;
  etc.num = num;
  return std::polymorphic<GameItem>(etc);
}

int ItemGameInstance::LoadSlotMax(const std::u16string& id) {
  int r = 200;
  auto info = LoadItemInfo(id, 0);
  if (info != nullptr && info->get_child(u"slotMax")) {
    r = static_cast<wz::Property<int>*>(info->get_child(u"slotMax"))->get();
  }
  return r;
}

int ItemGameInstance::LoadItemNum(std::polymorphic<GameItem>& itm) {
  int num = 0;
  switch (itm->type) {
    case ItemEnum::kConsume: {
      auto& consume = static_cast<GameConsumeItem&>(*itm);
      num = consume.num;
      break;
    }
    case ItemEnum::kEtc: {
      auto& etc = static_cast<GameEtcItem&>(*itm);
      num = etc.num;
      break;
    }
    default: {
      break;
    }
  }
  return num;
}

void ItemGameInstance::SetItemNum(std::polymorphic<GameItem>& itm, int num) {
  switch (itm->type) {
    case ItemEnum::kConsume: {
      auto& consume = static_cast<GameConsumeItem&>(*itm);
      consume.num = num;
      break;
    }
    case ItemEnum::kEtc: {
      auto& etc = static_cast<GameEtcItem&>(*itm);
      etc.num = num;
      break;
    }
    default: {
      break;
    }
  }
  QuestGameInstance::UpdateCheckItem();
  return;
}

void ItemGameInstance::AddItemNum(std::polymorphic<GameItem>& item, int num) {
  switch (item->type) {
    case ItemEnum::kConsume: {
      auto& consume = static_cast<GameConsumeItem&>(*item);
      consume.num += num;
      break;
    }
    case ItemEnum::kEtc: {
      auto& etc = static_cast<GameEtcItem&>(*item);
      etc.num += num;
      break;
    }
    default: {
      break;
    }
  }
  QuestGameInstance::UpdateCheckItem();
  return;
}

void ItemGameInstance::DecItemNum(std::polymorphic<GameItem>& item, int num) {
  switch (item->type) {
    case ItemEnum::kEquip: {
      item = std::polymorphic<GameItem>(GameEquipItem{});
      break;
    }
    case ItemEnum::kConsume: {
      auto& consume = static_cast<GameConsumeItem&>(*item);
      if (consume.num >= num) {
        consume.num = consume.num - num;
      } else {
        consume.num = 0;
      }
      if (consume.num == 0) {
        if (!item->id.starts_with(u"0207")) {
          item = std::polymorphic<GameItem>(GameConsumeItem{});
        }
      }
      break;
    }
    case ItemEnum::kEtc: {
      auto& etc = static_cast<GameEtcItem&>(*item);
      etc.num = etc.num - num;
      if (etc.num == 0) {
        item = std::polymorphic<GameItem>(GameEtcItem{});
      }
      break;
    }
    case ItemEnum::kInstall: {
      item = std::polymorphic<GameItem>(GameInstallItem{});
      break;
    }
    case ItemEnum::kCash: {
      item = std::polymorphic<GameItem>(GameCashItem{});
      break;
    }
    default: {
      break;
    }
  }
  QuestGameInstance::UpdateCheckItem();
  return;
}

void ItemGameInstance::DecItemById(const std::u16string& id) {
  auto& cash = PackageGameInstance::data_[static_cast<int>(ItemEnum::kCash)];
  for (auto& itm : cash) {
    if (itm->id == id) {
      DecItemNum(itm, 1);
      return;
    }
  }
}

void ItemGameInstance::UseMorphItem(const std::u16string& id,
                                    GameCharacter& g_character) {
  // 095：变身与骑乘互斥，骑乘中不能使用变身药水。
  if (!g_character.mount.empty()) {
    return;
  }
  auto info = ItemGameInstance::LoadItemInfo(id, 0);
  info = info->find(u"../spec");
  auto morph =
      static_cast<wz::Property<int>*>(info->get_child(u"morph"))->get();
  auto tmp = std::format("{:04d}", morph);
  g_character.morph = {tmp.begin(), tmp.end()};

  g_character.action_index = 0;
  g_character.action_time = 0;
  if (&g_character == &CharacterGameInstance::self_) {
    CharacterLogicSystem::RunUnsitChair(g_character);

    fbs::StateT st;
    st.state = fbs::StateEnum_BUFF_ITEM;
    st.val = std::stoi(std::string{id.begin(), id.end()});
    st.sub_val = 1;
    CharacterLogicSystem::ccs_.payload.push_back(
        std::make_unique<fbs::StateT>(st));

    st.state = fbs::StateEnum_ITEM_USE;
    st.sub_val = 1;
    CharacterLogicSystem::ccs_.payload.push_back(
        std::make_unique<fbs::StateT>(st));

    ServerCharacterInstance::HandleMorphUse(g_character, true);
  }
}

void ItemGameInstance::UnuseMorphItem(const std::u16string& id,
                                      GameCharacter& g_character) {
  g_character.morph = u"";
  g_character.action_index = 0;
  g_character.action_time = 0;

  if (&g_character == &CharacterGameInstance::self_) {
    fbs::StateT st;
    st.state = fbs::StateEnum_BUFF_ITEM;
    st.val = std::stoi(std::string{id.begin(), id.end()});
    st.sub_val = 0;

    auto& ccs = CharacterLogicSystem::ccs_;
    ccs.payload.push_back(std::make_unique<fbs::StateT>(st));

    st.state = fbs::StateEnum_ITEM_USE;
    st.sub_val = 0;
    CharacterLogicSystem::ccs_.payload.push_back(
        std::make_unique<fbs::StateT>(st));

    ServerCharacterInstance::HandleMorphUse(g_character, false);
  }
}

void ItemGameInstance::UnuseBuffItem(const std::u16string& id) {
  for (auto& gib : buff_) {
    if (gib.id == id) {
      // 百分比属性按使用时的快照扣除，不依赖 item 数据能否解析。
      if (gib.mhp_bonus != 0) {
        CharacterStatGameInstance::SetItemHp(
            CharacterStatGameInstance::GetItemHp() - gib.mhp_bonus);
      }
      if (gib.mmp_bonus != 0) {
        CharacterStatGameInstance::SetItemMp(
            CharacterStatGameInstance::GetItemMp() - gib.mmp_bonus);
      }
      auto info = ItemGameInstance::LoadItemInfo(id, 0);
      if (info != nullptr) {
        info = info->find(u"../spec");
      }
      if (info == nullptr) {
        break;
      }
      if (info->get_child(u"pad")) {
        auto pad =
            static_cast<wz::Property<int>*>(info->get_child(u"pad"))->get();
        auto itm_atk = CharacterStatGameInstance::GetItemAttack();
        CharacterStatGameInstance::SetItemAttack(itm_atk - pad);
      }
      if (info->get_child(u"mad")) {
        auto mad =
            static_cast<wz::Property<int>*>(info->get_child(u"mad"))->get();
        auto itm_magic = CharacterStatGameInstance::GetItemMagic();
        CharacterStatGameInstance::SetItemMagic(itm_magic - mad);
      }
      if (info->get_child(u"acc")) {
        auto acc =
            static_cast<wz::Property<int>*>(info->get_child(u"acc"))->get();
        auto itm_acc = CharacterStatGameInstance::GetItemAcc();
        CharacterStatGameInstance::SetItemAcc(itm_acc - acc);
      }
      if (info->get_child(u"eva")) {
        auto eva =
            static_cast<wz::Property<int>*>(info->get_child(u"eva"))->get();
        auto itm_avd = CharacterStatGameInstance::GetItemAvd();
        CharacterStatGameInstance::SetItemAvd(itm_avd - eva);
      }
      if (info->get_child(u"speed")) {
        auto speed =
            static_cast<wz::Property<int>*>(info->get_child(u"speed"))->get();
        CharacterStatGameInstance::SetItemSpeed(
            CharacterStatGameInstance::GetItemSpeed() - speed);
      }
      if (info->get_child(u"jump")) {
        auto jump =
            static_cast<wz::Property<int>*>(info->get_child(u"jump"))->get();
        CharacterStatGameInstance::SetItemJump(
            CharacterStatGameInstance::GetItemJump() - jump);
      }
      if (info->get_child(u"pdd")) {
        auto pdd =
            static_cast<wz::Property<int>*>(info->get_child(u"pdd"))->get();
        CharacterStatGameInstance::SetItemDef(
            CharacterStatGameInstance::GetItemDef() - pdd);
      }
      if (info->get_child(u"mdd")) {
        auto mdd =
            static_cast<wz::Property<int>*>(info->get_child(u"mdd"))->get();
        CharacterStatGameInstance::SetItemMdef(
            CharacterStatGameInstance::GetItemMdef() - mdd);
      }
      if (info->get_child(u"mhp")) {
        auto mhp =
            static_cast<wz::Property<int>*>(info->get_child(u"mhp"))->get();
        CharacterStatGameInstance::SetItemHp(
            CharacterStatGameInstance::GetItemHp() - mhp);
      }
      if (info->get_child(u"mmp")) {
        auto mmp =
            static_cast<wz::Property<int>*>(info->get_child(u"mmp"))->get();
        CharacterStatGameInstance::SetItemMp(
            CharacterStatGameInstance::GetItemMp() - mmp);
      }
      if (info->get_child(u"crt")) {
        auto crt =
            static_cast<wz::Property<int>*>(info->get_child(u"crt"))->get();
        auto sf_crt = CharacterStatGameInstance::GetCritDamage();
        CharacterStatGameInstance::SetCritDamage(sf_crt - crt);
      }
      if (info->get_child(u"damage")) {
        auto damage =
            static_cast<wz::Property<int>*>(info->get_child(u"damage"))->get();
        CharacterStatGameInstance::SetItemDamage(
            CharacterStatGameInstance::GetItemDamage() - damage);
      }
      if (info->get_child(u"expBuff")) {
        auto exp_buff =
            static_cast<wz::Property<int>*>(info->get_child(u"expBuff"))->get();
        auto itm_exp = CharacterStatGameInstance::GetItemExp();
        CharacterStatGameInstance::SetItemExp(itm_exp - exp_buff);
      }
      if (info->get_child(u"morph")) {
        auto& sf = CharacterGameInstance::self_;
        UnuseMorphItem(id, sf);
      }
      break;
    }
  }
  std::erase_if(buff_, [id](const auto& buff) { return id == buff.id; });
}

bool ItemGameInstance::UseBuffItem(std::polymorphic<GameItem>& itm) {
  auto id = itm->id;
  auto info = ItemGameInstance::LoadItemInfo(id, 0);
  if (info == nullptr) {
    return false;
  }
  info = info->find(u"../spec");
  if (info == nullptr) {
    return false;
  }
  if (!info->get_child(u"time")) {
    return false;
  }
  bool r = false;

  // 095：同属性组的消耗类 buff 互斥（后用的覆盖先用的），
  // 否则 ATT/魔法/速度等药水会无限堆叠，变身还会在旧 buff 到期时被清掉。
  std::vector<std::u16string> conflicts;
  for (const auto& b : buff_) {
    auto* old = LoadItemInfo(b.id, 0);
    old = old != nullptr ? old->find(u"../spec") : nullptr;
    if (old == nullptr) {
      continue;
    }
    for (auto key : {u"pad", u"mad", u"acc", u"eva", u"speed", u"crt",
                     u"expBuff", u"morph", u"jump", u"pdd", u"mdd",
                     u"mhp", u"mmp", u"mhpR", u"mmpR", u"damage"}) {
      if (info->get_child(key) != nullptr && old->get_child(key) != nullptr) {
        conflicts.push_back(b.id);
        break;
      }
    }
  }
  for (const auto& conflict : conflicts) {
    UnuseBuffItem(conflict);
  }
  UnuseBuffItem(itm->id);
  auto time = static_cast<wz::Property<int>*>(info->get_child(u"time"))->get();
  if (info->get_child(u"pad")) {
    auto pad = static_cast<wz::Property<int>*>(info->get_child(u"pad"))->get();
    auto itm_atk = CharacterStatGameInstance::GetItemAttack();
    CharacterStatGameInstance::SetItemAttack(itm_atk + pad);
    r = true;
  }
  if (info->get_child(u"mad")) {
    auto mad = static_cast<wz::Property<int>*>(info->get_child(u"mad"))->get();
    auto itm_mag = CharacterStatGameInstance::GetItemMagic();
    CharacterStatGameInstance::SetItemMagic(itm_mag + mad);
    r = true;
  }
  if (info->get_child(u"acc")) {
    auto acc = static_cast<wz::Property<int>*>(info->get_child(u"acc"))->get();
    auto itm_acc = CharacterStatGameInstance::GetItemAcc();
    CharacterStatGameInstance::SetItemAcc(acc + itm_acc);
    r = true;
  }
  if (info->get_child(u"eva")) {
    auto eva = static_cast<wz::Property<int>*>(info->get_child(u"eva"))->get();
    auto itm_avd = CharacterStatGameInstance::GetItemAvd();
    CharacterStatGameInstance::SetItemAvd(itm_avd + eva);
    r = true;
  }
  if (info->get_child(u"speed")) {
    auto speed = static_cast<wz::Property<int>*>(info->get_child(u"speed"))->get();
    CharacterStatGameInstance::SetItemSpeed(
        CharacterStatGameInstance::GetItemSpeed() + speed);
    r = true;
  }
  if (info->get_child(u"jump")) {
    auto jump =
        static_cast<wz::Property<int>*>(info->get_child(u"jump"))->get();
    CharacterStatGameInstance::SetItemJump(
        CharacterStatGameInstance::GetItemJump() + jump);
    r = true;
  }
  if (info->get_child(u"pdd")) {
    auto pdd = static_cast<wz::Property<int>*>(info->get_child(u"pdd"))->get();
    CharacterStatGameInstance::SetItemDef(
        CharacterStatGameInstance::GetItemDef() + pdd);
    r = true;
  }
  if (info->get_child(u"mdd")) {
    auto mdd = static_cast<wz::Property<int>*>(info->get_child(u"mdd"))->get();
    CharacterStatGameInstance::SetItemMdef(
        CharacterStatGameInstance::GetItemMdef() + mdd);
    r = true;
  }
  if (info->get_child(u"mhp")) {
    auto mhp = static_cast<wz::Property<int>*>(info->get_child(u"mhp"))->get();
    CharacterStatGameInstance::SetItemHp(
        CharacterStatGameInstance::GetItemHp() + mhp);
    r = true;
  }
  if (info->get_child(u"mmp")) {
    auto mmp = static_cast<wz::Property<int>*>(info->get_child(u"mmp"))->get();
    CharacterStatGameInstance::SetItemMp(
        CharacterStatGameInstance::GetItemMp() + mmp);
    r = true;
  }
  int64_t mhp_bonus = 0;
  int64_t mmp_bonus = 0;
  if (info->get_child(u"mhpR")) {
    auto rate =
        static_cast<wz::Property<int>*>(info->get_child(u"mhpR"))->get();
    auto base = std::max<int64_t>(
        0, CharacterStatGameInstance::GetHpMax() -
               CharacterStatGameInstance::GetItemHp());
    mhp_bonus = base * rate / 100;
    if (mhp_bonus != 0) {
      CharacterStatGameInstance::SetItemHp(
          CharacterStatGameInstance::GetItemHp() + mhp_bonus);
    }
    r = true;
  }
  if (info->get_child(u"mmpR")) {
    auto rate =
        static_cast<wz::Property<int>*>(info->get_child(u"mmpR"))->get();
    auto base = std::max<int64_t>(
        0, CharacterStatGameInstance::GetMpMax() -
               CharacterStatGameInstance::GetItemMp());
    mmp_bonus = base * rate / 100;
    if (mmp_bonus != 0) {
      CharacterStatGameInstance::SetItemMp(
          CharacterStatGameInstance::GetItemMp() + mmp_bonus);
    }
    r = true;
  }
  if (info->get_child(u"crt")) {
    auto crt = static_cast<wz::Property<int>*>(info->get_child(u"crt"))->get();
    auto sf_crt = CharacterStatGameInstance::GetCritDamage();
    CharacterStatGameInstance::SetCritDamage(sf_crt + crt);
    r = true;
  }
  if (info->get_child(u"damage")) {
    auto damage =
        static_cast<wz::Property<int>*>(info->get_child(u"damage"))->get();
    CharacterStatGameInstance::SetItemDamage(
        CharacterStatGameInstance::GetItemDamage() + damage);
    r = true;
  }
  if (info->get_child(u"expBuff")) {
    auto exp_buff =
        static_cast<wz::Property<int>*>(info->get_child(u"expBuff"))->get();
    auto itm_exp = CharacterStatGameInstance::GetItemExp();
    CharacterStatGameInstance::SetItemExp(itm_exp + exp_buff);
    r = true;
  }
  if (info->get_child(u"morph")) {
    UseMorphItem(id, CharacterGameInstance::self_);
    r = true;
  }
  if (!r) {
    // spec 里只有 time 而没有可应用的属性：不进入 buff 列表，也不消耗道具。
    return false;
  }
  GameItemBuff gib;
  gib.destroy = Window::dt_now_ + time;
  gib.id = id;
  gib.mhp_bonus = mhp_bonus;
  gib.mmp_bonus = mmp_bonus;

  buff_.emplace_back(gib);
  return true;
}

bool ItemGameInstance::UseMobItem(std::polymorphic<GameItem>& itm) {
  bool r = false;
  if (CharacterLogicSystem::self_fh_ == 0) {
    return false;
  }
  auto info = ItemGameInstance::LoadItemInfo(itm->id, 0);
  info = info->find(u"../mob");
  if (info) {
    auto& g_character = CharacterGameInstance::self_;
    for (auto [k, v] : *info->get_children()) {
      auto mob_id =
          static_cast<wz::Property<int>*>(v[0]->get_child(u"id"))->get();
      auto mob_num = 1;
      if (auto* count = v[0]->get_child(u"count");
          count != nullptr && count->type == wz::Type::Int) {
        mob_num = std::max(1, static_cast<wz::Property<int>*>(count)->get());
      }
      fbs::ClientCreateMobT ccm;
      ccm.map_id = SceneSystemInstance::map_id_;
      for (int i = 0; i < mob_num; i++) {
        ccm.mobs.emplace_back(std::make_unique<fbs::MobT>());
        auto& mob = ccm.mobs.back();
        mob->mob_id = mob_id;
        mob->state = std::make_unique<fbs::LifeStateT>(fbs::LifeStateT{
            .x = g_character.pos.x +
                 static_cast<float>(static_cast<int>(std::rand() % 161) - 80),
            .y = g_character.pos.y,
            .action_animate = true,
            .page = g_character.page,
            .fh = static_cast<int16_t>(CharacterLogicSystem::self_fh_),
        });
      }
      ClientRequest::SendToHost(ccm);
      r = true;
    }
  }
  return r;
}

bool ItemGameInstance::UseConsumeItem(const std::u16string& id) {
  bool r = false;
  auto info = ItemGameInstance::LoadItemInfo(id, 0);
  info = info->find(u"../spec");
  if (info == nullptr) {
    return false;
  }
  // 宠物食品（095：spec 里列出可喂的宠物 id + incRepleteness/incTameness）。
  {
    auto food_id = id;
    if (food_id.size() < 8) {
      food_id.insert(0, 8 - food_id.size(), u'0');
    }
    if (PetGameInstance::Feed(food_id)) {
      r = true;
    }
  }
  const int64_t hp_boost =
      100 + static_cast<int64_t>(CharacterStatGameInstance::GetHpRecoveryRate());
  const int64_t mp_boost =
      100 + static_cast<int64_t>(CharacterStatGameInstance::GetMpRecoveryRate());
  if (info->get_child(u"hp")) {
    auto hp = static_cast<wz::Property<int>*>(info->get_child(u"hp"))->get();
    auto sf_hp = CharacterStatGameInstance::GetHp();
    CharacterStatGameInstance::SetHp(
        sf_hp + static_cast<int32_t>(hp * hp_boost / 100));
    r = true;
  }
  if (info->get_child(u"hpR")) {
    auto hp_ratio =
        static_cast<wz::Property<int>*>(info->get_child(u"hpR"))->get();
    auto sf_hp_max = CharacterStatGameInstance::GetHpMax();
    auto sf_hp = CharacterStatGameInstance::GetHp();
    const int64_t heal =
        static_cast<int64_t>(sf_hp_max) * hp_ratio / 100 * hp_boost / 100;
    CharacterStatGameInstance::SetHp(sf_hp + static_cast<int32_t>(heal));
    r = true;
  }
  if (info->get_child(u"mp")) {
    auto mp = static_cast<wz::Property<int>*>(info->get_child(u"mp"))->get();
    auto sf_mp = CharacterStatGameInstance::GetMp();
    CharacterStatGameInstance::SetMp(
        sf_mp + static_cast<int32_t>(mp * mp_boost / 100));
    r = true;
  }
  if (info->get_child(u"mpR")) {
    auto mp_ratio =
        static_cast<wz::Property<int>*>(info->get_child(u"mpR"))->get();
    auto sf_mp_max = CharacterStatGameInstance::GetMpMax();
    auto sf_mp = CharacterStatGameInstance::GetMp();
    const int64_t gain =
        static_cast<int64_t>(sf_mp_max) * mp_ratio / 100 * mp_boost / 100;
    CharacterStatGameInstance::SetMp(sf_mp + static_cast<int32_t>(gain));
    r = true;
  }
  if (info->get_child(u"moveTo")) {
    auto move_to =
        static_cast<wz::Property<int>*>(info->get_child(u"moveTo"))->get();
    SceneSystemInstance::EnterPrepare(move_to, u"sp", 0);
    r = true;
  }
  if (info->get_child(u"exp")) {
    auto exp = static_cast<wz::Property<int>*>(info->get_child(u"exp"))->get();
    CharacterStatGameInstance::AddExp(exp);
    r = true;
  }
  return r;
}

bool ItemGameInstance::UseConsumeItem(std::polymorphic<GameItem>& itm) {
  bool r = false;
  if (UseBuffItem(itm)) {
    r = true;
  }
  if (UseConsumeItem(itm->id)) {
    r = true;
  }
  if (UseMobItem(itm)) {
    r = true;
  }
  return r;
}

void ItemGameInstance::UseItem(std::polymorphic<GameItem>& itm) {
  auto item_type = ItemGameInstance::LoadItemType(itm->id);
  // 095：宠物道具的激活/收回没有使用冷却，连续点击可以立即切换。
  if (item_type == u"Pet") {
    PetGameInstance::Toggle(itm->id);
    return;
  }

  const uint64_t now = Window::dt_now_;
  if (now < item_use_cooldown_) {
    return;
  }
  // 095 的请求间隔按类型区分：药水 200ms，其余道具（状态/卷轴类）600ms。
  int64_t use_cooldown = 600;
  if (item_type == u"Consume") {
    auto* spec = LoadItemInfo(itm->id, 0);
    spec = spec != nullptr ? spec->find(u"../spec") : nullptr;
    if (spec != nullptr &&
        (spec->get_child(u"hp") != nullptr || spec->get_child(u"hpR") != nullptr ||
         spec->get_child(u"mp") != nullptr || spec->get_child(u"mpR") != nullptr)) {
      use_cooldown = 200;
    }
  }
  item_use_cooldown_ = now + use_cooldown;

  auto& g_character = CharacterGameInstance::self_;
  // 095：死亡状态不能使用消耗/状态类道具。
  if (CharacterLogicSystem::LoadActionType(g_character) ==
      CharacterLogicSystem::ActionEnum::kDead) {
    return;
  }
  auto self_sit_cooldown = CharacterLogicSystem::self_sit_cooldown_;
  auto map_id = SceneSystemInstance::map_id_;
  auto cannot_here = [] {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopNotice;
    NoticeUiSystem::notice_text_ =
        WzResource::LoadMSString(u"cannotHere", u"在这个地图做不了。");
    NoticeUiSystem::Open();
  };

  // 095 SendConsumeCashItemUseRequest：fieldLimit bit18 禁用现金道具。
  if (item_type == u"Cash" && MapInfoGameInstance::IsCashItemDisabled(map_id)) {
    cannot_here();
    return;
  }

  // 095 HiredShop：雇佣商人执照 0503000x → 摆摊设置窗（雇佣模式）。
  if (item_type == u"Cash" && itm->id.starts_with(u"0503")) {
    PlayerShopUiSystem::OpenHired(static_cast<uint32_t>(
        std::stoul(std::string{itm->id.begin(), itm->id.end()})));
    return;
  }

  // 095 SendConsumeCashItemUseRequest type=16：天气现金道具先弹输入框，
  // 由服务器 BlowWeather 广播后再显示 FloatNotice。
  // 095 TransientLayer_Exist：场上已有天气特效时拒绝再次使用。
  if (item_type == u"Cash" && itm->id.starts_with(u"0512")) {
    if (WeatherGameInstance::Exists()) {
      NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopNotice;
      NoticeUiSystem::notice_text_ = WzResource::LoadMSString(
          u"weatherExist", u"当前地图已经有天气效果了。");
      NoticeUiSystem::Open();
      return;
    }
    WeatherDlgUiSystem::Open(itm->id);
    return;
  }

  if (item_type == u"TamingMob") {
    // 095：使用坐骑道具 = 上骑 / 再用下骑（SetRidingVehicle 语义）。
    if (CharacterLogicSystem::IsAlert()) {
      return;  // 受击/攻击中不能上骑（095 同款限制）
    }
    if (!g_character.mount.empty()) {
      CharacterLogicSystem::RunUnmount(g_character);
    } else {
      CharacterLogicSystem::RunMount(g_character, itm->id);
    }
    return;
  }

  if (item_type == u"Ins") {
    // 095 SendCreateMiniGameRequest：fieldLimit bit7 禁用小游戏。
    if ((IsMatchCardsTable(itm->id) || IsMinigameTable(itm->id)) &&
        MapInfoGameInstance::IsMinigameDisabled(map_id)) {
      cannot_here();
      return;
    }
    if (IsMatchCardsTable(itm->id)) {
      // 095 CMiniRoomBaseDlg：开房前先弹房间设置（公开/密码），确认后
      // 进入/创建房间，无同图空房时自己成为房主。
      MinigameSetupUiSystem::Open(1);
      return;
    }
    if (IsMinigameTable(itm->id)) {
      MinigameSetupUiSystem::Open(0);
      return;
    }
    if (itm->id.starts_with(u"0301") && self_sit_cooldown < Window::dt_now_) {
      // 095：alert 期间不能使用椅子。
      if (CharacterLogicSystem::IsAlert()) {
        return;
      }
      if (!g_character.morph.empty()) {
        return;
      }
      AudioGameInstance::PlayNow(u"Item.img/" + itm->id);
      auto action_type = CharacterLogicSystem::LoadActionType(g_character);
      switch (action_type) {
        case CharacterLogicSystem::ActionEnum::kStand: {
          CharacterLogicSystem::RunSitChair(g_character, itm->id);
          break;
        }
        case CharacterLogicSystem::ActionEnum::kSit: {
          CharacterLogicSystem::RunUnsitChair(g_character);
          break;
        }
        default: {
          break;
        }
      }
      return;
    }
  } else if (item_type == u"Consume") {
    if (UseConsumeItem(itm)) {
      std::u16string path = u"Item.img/" + itm->id + u"/Use";
      auto audio_node = WzResource::sound_->find(path);
      if (audio_node) {
        AudioGameInstance::PlayNow(path);
      }
      DecItemNum(itm, 1);
    }
  }
  return;
}

void ItemGameInstance::EnsureMountItem() {
  auto& items = PackageGameInstance::data_[(int)ItemEnum::kInstall];
  for (auto& itm : items) {
    if (itm->id.starts_with(u"0190") || itm->id.starts_with(u"0191") ||
        itm->id.starts_with(u"0193")) {
      return;
    }
  }
  for (auto& slot : items) {
    if (slot->id.empty()) {
      slot = LoadItem(u"01902000", 1);
      return;
    }
  }
}

}  // namespace sdlms
