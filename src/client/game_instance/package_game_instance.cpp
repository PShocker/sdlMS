#include "package_game_instance.h"
#include "character_game_instance.h"
#include "equip_game_instance.h"
#include "item_game_instance.h"
#include "src/client/game/game_item.h"
#include "src/client/system/ui/package_ui_system.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>

void package_game_instance::load(const character_save &cs) {
  for (auto &d : data) {
    d.clear(); // 先清空
  }
  game_consume_item gci;
  // gci.id = u"02040000";
  // gci.id = u"02070008";
  gci.id = u"02210001";
  gci.num = 3;

  data[(int)item_enum::equip].assign(
      96, std::polymorphic<game_item>(game_equip_item{}));
  data[(int)item_enum::consume].assign(96, std::polymorphic<game_item>(gci));
  data[(int)item_enum::install].assign(
      96, std::polymorphic<game_item>(game_install_item{}));
  data[(int)item_enum::etc].assign(
      96, std::polymorphic<game_item>(game_etc_item{}));
  data[(int)item_enum::cash].assign(
      96, std::polymorphic<game_item>(game_cash_item{}));
  data[(int)item_enum::deco].assign(
      96, std::polymorphic<game_item>(game_deco_item{}));

  gci.id = u"02000000";
  gci.num = 1000;
  data[(int)item_enum::consume][0] = std::polymorphic<game_item>(gci);

  meso = 1000000000;
  data[0][0]->id = u"01472012";

  for (auto &pkg : cs.package) {
    auto type = (int)pkg.val->type;
    data[type][pkg.index] = pkg.val;
  }
  // meso = cs.meso;
}

std::vector<int> package_game_instance::load_empty_index(uint32_t tab) {
  std::vector<int> r;
  auto &d = package_game_instance::data[tab];
  for (int32_t i = 0; i < d.size(); i++) {
    if (d[i]->id.empty()) {
      r.push_back(i);
    }
  }
  return r;
}

std::vector<int>
package_game_instance::load_empty_index(std::polymorphic<game_item> &item) {
  if (item->id == u"00000000") {
    // meso
    return {0};
  }
  if (!(item->type == item_enum::consume || item->type == item_enum::etc)) {
    return load_empty_index((int)item->type);
  }
  std::vector<int> blank;
  auto num = item_game_instance::load_item_num(item);
  std::vector<std::polymorphic<game_item>> *r;
  r = &package_game_instance::data[(int)item->type];
  auto slot_max = item_game_instance::load_slot_max(item->id);
  std::vector<int32_t> same_slots;
  std::vector<int32_t> add_slots;
  for (int32_t i = 0; i < r->size(); i++) {
    auto itm = r->at(i);
    if (itm->id == item->id) {
      same_slots.push_back(i);
    } else if (itm->id.empty()) {
      add_slots.push_back(i);
    }
  }
  same_slots.append_range(add_slots);
  for (auto i : same_slots) {
    auto itm = r->at(i);
    if (itm->id == item->id) {
      auto itm_num = item_game_instance::load_item_num(itm);
      if (itm_num < slot_max) {
        num = num - (slot_max - itm_num);
      }
      blank.push_back(i);
    } else if (itm->id.empty()) {
      num -= slot_max;
      blank.push_back(i);
    }
    if (num <= 0) {
      break;
    }
  }
  if (num <= 0) {
    return blank;
  } else {
    return {};
  }
}

std::polymorphic<game_item> *
package_game_instance::load_item(const std::u16string &id) {
  auto type = item_game_instance::load_item_type(id);
  std::vector<std::polymorphic<game_item>> *r;
  if (type == u"Cash" || type == u"Pet") {
    auto r = package_game_instance::data[(int)item_enum::cash];
  } else if (type == u"Consume") {
    auto r = package_game_instance::data[(int)item_enum::consume];
  } else if (type == u"Install") {
    auto r = package_game_instance::data[(int)item_enum::install];
  }
  for (auto &itm : *r) {
    if (itm->id == id) {
      return &itm;
    }
  }
  return nullptr;
}

std::polymorphic<game_item> *package_game_instance::load_active_ball() {
  auto &sf = character_game_instance::self;
  std::u16string pre;
  auto weapon_type = equip_game_instance::load_weapon_type(sf);
  switch (weapon_type) {
  case equip_game_instance::weapon_type::BOW: {
    break;
  }
  case equip_game_instance::weapon_type::CROSSBOW: {
    break;
  }
  case equip_game_instance::weapon_type::CLAW: {
    pre = u"0207";
    break;
  }
  default: {
    return nullptr;
    break;
  }
  }
  auto &r = package_game_instance::data[(int)item_enum::consume];
  for (auto &itm : r) {
    if (itm->id.starts_with(pre)) {
      auto num = item_game_instance::load_item_num(itm);
      if (num > 0) {
        return &itm;
      }
    }
  }
  return nullptr;
}

std::u16string package_game_instance::load_active_cash_ball() {
  auto &sf = character_game_instance::self;
  std::u16string pre;
  auto weapon_type = equip_game_instance::load_weapon_type(sf);
  switch (weapon_type) {
  case equip_game_instance::weapon_type::BOW: {
    break;
  }
  case equip_game_instance::weapon_type::CROSSBOW: {
    break;
  }
  case equip_game_instance::weapon_type::CLAW: {
    pre = u"0502";
    break;
  }
  default: {
    return u"";
    break;
  }
  }
  auto &r = package_game_instance::data[(int)item_enum::cash];
  for (auto &itm : r) {
    if (itm->id.starts_with(pre)) {
      return itm->id;
    }
  }
  return u"";
}

uint32_t package_game_instance::load_item_num(const std::u16string &id) {
  uint32_t r = 0;
  auto itm = item_game_instance::load_item(id, 1);
  auto &p = package_game_instance::data[(int)itm->type];
  for (auto &i : p) {
    if (i->id == id) {
      auto num = item_game_instance::load_item_num(i);
      r += num;
    }
  }
  return r;
}

int package_game_instance::add_item_slot(std::polymorphic<game_item> &item,
                                         int i) {
  auto num = item_game_instance::load_item_num(item);
  auto &p = package_game_instance::data[(int)item->type];
  auto slot_max = item_game_instance::load_slot_max(item->id);
  auto &itm = p.at(i);
  itm->id = item->id;
  auto itm_num = item_game_instance::load_item_num(itm);
  num = std::min((int)num + itm_num, slot_max);
  auto dn = num - itm_num;
  item_game_instance::add_item_num(itm, dn);
  item_game_instance::dec_item_num(item, dn);
  return dn;
}

std::optional<int>
package_game_instance::add_item(std::polymorphic<game_item> &item) {
  auto b = load_empty_index(item);
  if (b.empty()) {
    return std::nullopt;
  }
  if (item->id == u"00000000") {
    auto num = item_game_instance::load_item_num(item);
    package_game_instance::meso += num;
    return 0;
  }
  std::optional<int> r;
  auto &p = package_game_instance::data[(int)item->type];
  switch (item->type) {
  case item_enum::consume:
  case item_enum::etc: {
    for (auto i : b) {
      if (add_item_slot(item, i)) {
        r = i;
      }
    }
    break;
  }
  default: {
    r = b[0];
    p[b[0]] = (item);
    break;
  }
  }
  return r;
}

std::optional<int>
package_game_instance::add_new_item(std::polymorphic<game_item> &item) {
  auto i = add_item(item);
  if (i.has_value()) {
    if (item->id != u"00000000") {
      package_ui_system::new_itm = {
          .type = item->type,
          .index = (uint16_t)(i.value()),
      };
    }
  }
  return i;
}