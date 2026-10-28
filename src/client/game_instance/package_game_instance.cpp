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

void PackageGameInstance::Load(const CharacterSaveData &cs) {
  for (auto &d : data) {
    d.clear(); // 先清空
  }
  GameConsumeItem gci;
  // gci.id = u"02040000";
  gci.id = u"02070005";
  // gci.id = u"02060000";
  gci.num = 3;

  data[(int)ItemEnum::kEquip].assign(
      96, std::polymorphic<GameItem>(GameEquipItem{}));
  data[(int)ItemEnum::kConsume].assign(96, std::polymorphic<GameItem>(gci));
  data[(int)ItemEnum::kInstall].assign(
      96, std::polymorphic<GameItem>(GameInstallItem{}));
  data[(int)ItemEnum::kEtc].assign(
      96, std::polymorphic<GameItem>(GameEtcItem{}));
  data[(int)ItemEnum::kCash].assign(
      96, std::polymorphic<GameItem>(GameCashItem{}));
  data[(int)ItemEnum::kDeco].assign(
      96, std::polymorphic<GameItem>(GameDecoItem{}));

  gci.id = u"02000000";
  gci.num = 1000;
  data[(int)ItemEnum::kConsume][0] = std::polymorphic<GameItem>(gci);

  GameEtcItem gei;
  gei.id = u"04031002";
  gei.num = 1;
  data[(int)ItemEnum::kEtc][0] = std::polymorphic<GameItem>(gei);

  GameInstallItem gii;
  gei.id = u"03010000";
  data[(int)ItemEnum::kInstall][0] = std::polymorphic<GameItem>(gei);

  meso = 1000000000;
  data[0][0]->id = u"01472012";

  for (auto &pkg : cs.package) {
    auto type = (int)pkg.val->type;
    data[type][pkg.index] = pkg.val;
  }
  // meso = cs.meso;
}

std::vector<int> PackageGameInstance::LoadEmptyIndex(uint32_t tab) {
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
PackageGameInstance::LoadEmptyIndex(std::polymorphic<GameItem> &item) {
  // 特殊：meso
  if (item->id == u"00000000") {
    return {0};
  }

  // 特殊：0207 前缀
  if (item->id.starts_with(u"0207")) {
    auto b = LoadEmptyIndex(static_cast<int>(item->type));
    return b.empty() ? std::vector<int>{} : std::vector<int>{b[0]};
  }

  // 非消耗品/其他类型：直接按 type 索引
  if (item->type != ItemEnum::kConsume && item->type != ItemEnum::kEtc) {
    return LoadEmptyIndex(static_cast<int>(item->type));
  }

  auto *r = &PackageGameInstance::data[static_cast<int>(item->type)];
  const auto slot_max = ItemGameInstance::LoadSlotMax(item->id);
  int num = ItemGameInstance::LoadItemNum(item);

  std::vector<int> blank;
  const size_t n = r->size();

  // 第一遍：收集同 id 且未满的槽位
  for (size_t i = 0; i < n && num > 0; ++i) {
    auto &itm = r->at(i);
    if (itm->id == item->id) {
      const auto itm_num = ItemGameInstance::LoadItemNum(itm);
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
PackageGameInstance::LoadItem(const std::u16string &id) {
  auto type = ItemGameInstance::LoadItemType(id);
  std::vector<std::polymorphic<GameItem>> *r;
  if (type == u"Cash" || type == u"Pet") {
    r = &PackageGameInstance::data[(int)ItemEnum::kCash];
  } else if (type == u"Consume") {
    r = &PackageGameInstance::data[(int)ItemEnum::kConsume];
  } else if (type == u"Ins") {
    r = &PackageGameInstance::data[(int)ItemEnum::kInstall];
  } else if (type == u"Etc") {
    r = &PackageGameInstance::data[(int)ItemEnum::kEtc];
  }
  for (auto &itm : *r) {
    if (itm->id == id) {
      return &itm;
    }
  }
  return nullptr;
}

std::polymorphic<GameItem> *PackageGameInstance::LoadActiveBall() {
  auto &sf = CharacterGameInstance::self;
  std::u16string pre;
  auto WeaponType = EquipGameInstance::LoadWeaponType(sf);
  switch (WeaponType) {
  case EquipGameInstance::WeaponType::kBow: {
    pre = u"02060";
    break;
  }
  case EquipGameInstance::WeaponType::kCrossbow: {
    pre = u"02061";
    break;
  }
  case EquipGameInstance::WeaponType::kClaw: {
    pre = u"0207";
    break;
  }
  default: {
    return nullptr;
    break;
  }
  }
  auto &r = PackageGameInstance::data[(int)ItemEnum::kConsume];
  for (auto &itm : r) {
    if (itm->id.starts_with(pre)) {
      auto num = ItemGameInstance::LoadItemNum(itm);
      if (num > 0) {
        return &itm;
      }
    }
  }
  return nullptr;
}

std::u16string PackageGameInstance::LoadActiveCashBall() {
  auto &sf = CharacterGameInstance::self;
  std::u16string pre;
  auto WeaponType = EquipGameInstance::LoadWeaponType(sf);
  switch (WeaponType) {
  case EquipGameInstance::WeaponType::kBow: {
    break;
  }
  case EquipGameInstance::WeaponType::kCrossbow: {
    break;
  }
  case EquipGameInstance::WeaponType::kClaw: {
    pre = u"0502";
    break;
  }
  default: {
    return u"";
    break;
  }
  }
  auto &r = PackageGameInstance::data[(int)ItemEnum::kCash];
  for (auto &itm : r) {
    if (itm->id.starts_with(pre)) {
      return itm->id;
    }
  }
  return u"";
}

uint32_t PackageGameInstance::LoadItemNum(const std::u16string &id) {
  uint32_t r = 0;
  auto itm = ItemGameInstance::LoadItem(id, 1);
  auto &p = PackageGameInstance::data[(int)itm->type];
  for (auto &i : p) {
    if (i->id == id) {
      auto num = ItemGameInstance::LoadItemNum(i);
      r += num;
    }
  }
  return r;
}

int PackageGameInstance::AddItemSlot(std::polymorphic<GameItem> &item,
                                         int i) {
  auto num = ItemGameInstance::LoadItemNum(item);
  auto &p = PackageGameInstance::data[(int)item->type];
  auto slot_max = ItemGameInstance::LoadSlotMax(item->id);
  auto &itm = p.at(i);
  itm->id = item->id;
  auto itm_num = ItemGameInstance::LoadItemNum(itm);
  num = std::min((int)num + itm_num, slot_max);
  auto dn = num - itm_num;
  ItemGameInstance::AddItemNum(itm, dn);
  ItemGameInstance::DecItemNum(item, dn);
  return dn;
}

std::optional<int>
PackageGameInstance::AddItem(std::polymorphic<GameItem> &item) {
  // 提前解引用，避免多次 -> 调用
  const auto type = item->type;

  // 特例：货币类道具（id 全零）
  if (item->id == u"00000000") {
    AddMeso(ItemGameInstance::LoadItemNum(item));
    return 0;
  }

  auto empty_slots = LoadEmptyIndex(item);
  if (empty_slots.empty()) {
    return std::nullopt;
  }

  std::optional<int> result;
  auto &slots = PackageGameInstance::data[static_cast<int>(type)];

  switch (type) {
  case ItemEnum::kConsume:
  case ItemEnum::kEtc: {
    // 逐个尝试放入空槽，直到成功一个即可
    for (const int idx : empty_slots) {
      if (AddItemSlot(item, idx)) {
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
    QuestGameInstance::UpdateCheckItem();
  }
  return result;
}

std::optional<int>
PackageGameInstance::AddNewItem(std::polymorphic<GameItem> &item) {
  auto i = AddItem(item);
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

bool PackageGameInstance::AddMeso(uint32_t num) {
  PackageGameInstance::meso += num;
  return true;
}