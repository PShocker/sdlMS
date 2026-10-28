#include "package_game_instance.h"
#include "character_game_instance.h"
#include "equip_game_instance.h"
#include "item_game_instance.h"
#include "quest_game_instance.h"
#include "src/client/game/game_item.h"
#include "src/client/system/ui/package_ui_system.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>

void PackageGameInstance::load(const CharacterSaveData &cs) {
  for (auto &d : data) {
    d.clear(); // 先清空
  }
  GameConsumeItem gci;
  // gci.id = u"02040000";
  gci.id = u"02070005";
  // gci.id = u"02060000";
  gci.num = 3;

  data[(int)ItemEnum::equip].assign(
      96, std::polymorphic<GameItem>(GameEquipItem{}));
  data[(int)ItemEnum::consume].assign(96, std::polymorphic<GameItem>(gci));
  data[(int)ItemEnum::install].assign(
      96, std::polymorphic<GameItem>(GameInstallItem{}));
  data[(int)ItemEnum::etc].assign(
      96, std::polymorphic<GameItem>(GameEtcItem{}));
  data[(int)ItemEnum::cash].assign(
      96, std::polymorphic<GameItem>(GameCashItem{}));
  data[(int)ItemEnum::deco].assign(
      96, std::polymorphic<GameItem>(GameDecoItem{}));

  gci.id = u"02000000";
  gci.num = 1000;
  data[(int)ItemEnum::consume][0] = std::polymorphic<GameItem>(gci);

  GameEtcItem gei;
  gei.id = u"04031002";
  gei.num = 1;
  data[(int)ItemEnum::etc][0] = std::polymorphic<GameItem>(gei);

  GameInstallItem gii;
  gei.id = u"03010000";
  data[(int)ItemEnum::install][0] = std::polymorphic<GameItem>(gei);

  meso = 1000000000;
  data[0][0]->id = u"01472012";

  for (auto &pkg : cs.package) {
    auto type = (int)pkg.val->type;
    data[type][pkg.index] = pkg.val;
  }
  // meso = cs.meso;
}

std::vector<int> PackageGameInstance::load_empty_index(uint32_t tab) {
  std::vector<int> r;
  auto &d = PackageGameInstance::data[tab];
  for (int32_t i = 0; i < d.size(); i++) {
    if (d[i]->id.empty()) {
      r.push_back(i);
    }
  }
  return r;
}

std::vector<int>
PackageGameInstance::load_empty_index(std::polymorphic<GameItem> &item) {
  // 特殊：meso
  if (item->id == u"00000000") {
    return {0};
  }

  // 特殊：0207 前缀
  if (item->id.starts_with(u"0207")) {
    auto b = load_empty_index(static_cast<int>(item->type));
    return b.empty() ? std::vector<int>{} : std::vector<int>{b[0]};
  }

  // 非消耗品/其他类型：直接按 type 索引
  if (item->type != ItemEnum::consume && item->type != ItemEnum::etc) {
    return load_empty_index(static_cast<int>(item->type));
  }

  auto *r = &PackageGameInstance::data[static_cast<int>(item->type)];
  const auto slot_max = ItemGameInstance::load_slot_max(item->id);
  int num = ItemGameInstance::load_item_num(item);

  std::vector<int> blank;
  const size_t n = r->size();

  // 第一遍：收集同 id 且未满的槽位
  for (size_t i = 0; i < n && num > 0; ++i) {
    auto &itm = r->at(i);
    if (itm->id == item->id) {
      const auto itm_num = ItemGameInstance::load_item_num(itm);
      if (itm_num >= slot_max)
        continue; // 已满，跳过
      num -= (slot_max - itm_num);
      blank.push_back(static_cast<int>(i));
    }
  }
  // 第二遍：若还没装满，继续找空槽位
  for (size_t i = 0; i < n && num > 0; ++i) {
    const auto &itm = r->at(i);
    if (itm->id.empty()) {
      num -= slot_max;
      blank.push_back(static_cast<int>(i));
    }
  }

  return num <= 0 ? blank : std::vector<int>{};
}

std::polymorphic<GameItem> *
PackageGameInstance::load_item(const std::u16string &id) {
  auto type = ItemGameInstance::load_item_type(id);
  std::vector<std::polymorphic<GameItem>> *r;
  if (type == u"Cash" || type == u"Pet") {
    r = &PackageGameInstance::data[(int)ItemEnum::cash];
  } else if (type == u"Consume") {
    r = &PackageGameInstance::data[(int)ItemEnum::consume];
  } else if (type == u"Ins") {
    r = &PackageGameInstance::data[(int)ItemEnum::install];
  } else if (type == u"Etc") {
    r = &PackageGameInstance::data[(int)ItemEnum::etc];
  }
  for (auto &itm : *r) {
    if (itm->id == id) {
      return &itm;
    }
  }
  return nullptr;
}

std::polymorphic<GameItem> *PackageGameInstance::load_active_ball() {
  auto &sf = CharacterGameInstance::self;
  std::u16string pre;
  auto WeaponType = EquipGameInstance::load_weapon_type(sf);
  switch (WeaponType) {
  case EquipGameInstance::WeaponType::BOW: {
    pre = u"02060";
    break;
  }
  case EquipGameInstance::WeaponType::CROSSBOW: {
    pre = u"02061";
    break;
  }
  case EquipGameInstance::WeaponType::CLAW: {
    pre = u"0207";
    break;
  }
  default: {
    return nullptr;
    break;
  }
  }
  auto &r = PackageGameInstance::data[(int)ItemEnum::consume];
  for (auto &itm : r) {
    if (itm->id.starts_with(pre)) {
      auto num = ItemGameInstance::load_item_num(itm);
      if (num > 0) {
        return &itm;
      }
    }
  }
  return nullptr;
}

std::u16string PackageGameInstance::load_active_cash_ball() {
  auto &sf = CharacterGameInstance::self;
  std::u16string pre;
  auto WeaponType = EquipGameInstance::load_weapon_type(sf);
  switch (WeaponType) {
  case EquipGameInstance::WeaponType::BOW: {
    break;
  }
  case EquipGameInstance::WeaponType::CROSSBOW: {
    break;
  }
  case EquipGameInstance::WeaponType::CLAW: {
    pre = u"0502";
    break;
  }
  default: {
    return u"";
    break;
  }
  }
  auto &r = PackageGameInstance::data[(int)ItemEnum::cash];
  for (auto &itm : r) {
    if (itm->id.starts_with(pre)) {
      return itm->id;
    }
  }
  return u"";
}

uint32_t PackageGameInstance::load_item_num(const std::u16string &id) {
  uint32_t r = 0;
  auto itm = ItemGameInstance::load_item(id, 1);
  auto &p = PackageGameInstance::data[(int)itm->type];
  for (auto &i : p) {
    if (i->id == id) {
      auto num = ItemGameInstance::load_item_num(i);
      r += num;
    }
  }
  return r;
}

int PackageGameInstance::add_item_slot(std::polymorphic<GameItem> &item,
                                         int i) {
  auto num = ItemGameInstance::load_item_num(item);
  auto &p = PackageGameInstance::data[(int)item->type];
  auto slot_max = ItemGameInstance::load_slot_max(item->id);
  auto &itm = p.at(i);
  itm->id = item->id;
  auto itm_num = ItemGameInstance::load_item_num(itm);
  num = std::min((int)num + itm_num, slot_max);
  auto dn = num - itm_num;
  ItemGameInstance::add_item_num(itm, dn);
  ItemGameInstance::dec_item_num(item, dn);
  return dn;
}

std::optional<int>
PackageGameInstance::add_item(std::polymorphic<GameItem> &item) {
  // 提前解引用，避免多次 -> 调用
  const auto type = item->type;

  // 特例：货币类道具（id 全零）
  if (item->id == u"00000000") {
    add_meso(ItemGameInstance::load_item_num(item));
    return 0;
  }

  auto empty_slots = load_empty_index(item);
  if (empty_slots.empty()) {
    return std::nullopt;
  }

  std::optional<int> result;
  auto &slots = PackageGameInstance::data[static_cast<int>(type)];

  switch (type) {
  case ItemEnum::consume:
  case ItemEnum::etc: {
    // 逐个尝试放入空槽，直到成功一个即可
    for (const int idx : empty_slots) {
      if (add_item_slot(item, idx)) {
        result = idx;
        break; // 成功即止，避免重复添加
      }
    }
    break;
  }
  default: {
    const int idx = empty_slots.front();
    slots[idx] = item;
    result = idx;
    break;
  }
  }
  // 仅在实际添加成功时更新任务检查
  if (result) {
    QuestGameInstance::update_check_item();
  }
  return result;
}

std::optional<int>
PackageGameInstance::add_new_item(std::polymorphic<GameItem> &item) {
  auto i = add_item(item);
  if (i.has_value()) {
    if (item->id != u"00000000") {
      PackageUiSystem::new_itm = {
          .type = item->type,
          .index = (uint16_t)(i.value()),
      };
    }
  }
  return i;
}

bool PackageGameInstance::add_meso(uint32_t num) {
  PackageGameInstance::meso += num;
  return true;
}