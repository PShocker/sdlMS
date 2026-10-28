#include "equip_game_instance.h"
#include "character_game_instance.h"
#include "item_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <ranges>
#include <string>
#include <vector>

bool EquipGameInstance::check_equip(const std::u16string &id) {
  auto r = id.substr(0, 2);
  if (r == u"01") {
    return true;
  }
  return false;
}

std::polymorphic<GameItem>
EquipGameInstance::load_item(const std::u16string &id) {
  GameEquipItem eqp;
  eqp.type = ItemEnum::equip;
  eqp.id = id;
  auto r = std::polymorphic<GameItem>(eqp);
  return r;
}

std::u16string EquipGameInstance::load_equip_type(const std::u16string &id) {
  auto result = id.substr(1, 3);
  static const std::flat_map<std::u16string, std::u16string> equip_type = {
      // Accessory
      {u"101", u"Accessory"},
      {u"102", u"Accessory"},
      {u"103", u"Accessory"},
      {u"112", u"Accessory"},
      {u"113", u"Accessory"},
      {u"114", u"Accessory"},
      {u"115", u"Accessory"},
      // Cap
      {u"100", u"Cap"},
      // Cape
      {u"110", u"Cape"},
      // Coat
      {u"104", u"Coat"},
      // Glove
      {u"108", u"Glove"},
      // Longcoat
      {u"105", u"Longcoat"},
      // Pants
      {u"106", u"Pants"},
      // PetEquip
      {u"180", u"PetEquip"},
      {u"181", u"PetEquip"},
      // Ring
      {u"111", u"Ring"},
      // Shield
      {u"109", u"Shield"},
      // Shoes
      {u"107", u"Shoes"},
      // TamingMob
      {u"190", u"TamingMob"},
      {u"191", u"TamingMob"},
      {u"193", u"TamingMob"},
      // Weapon
      {u"130", u"Weapon"},
      {u"131", u"Weapon"},
      {u"132", u"Weapon"},
      {u"133", u"Weapon"},
      {u"137", u"Weapon"},
      {u"138", u"Weapon"},
      {u"140", u"Weapon"},
      {u"141", u"Weapon"},
      {u"142", u"Weapon"},
      {u"143", u"Weapon"},
      {u"144", u"Weapon"},
      {u"145", u"Weapon"},
      {u"146", u"Weapon"},
      {u"147", u"Weapon"},
      {u"148", u"Weapon"},
      {u"149", u"Weapon"},
      {u"160", u"Weapon"},
      {u"170", u"Weapon"},
      {u"000", u"Body"},
      {u"001", u"Head"},
      {u"002", u"Face"},
      {u"003", u"Hair"},
  };
  if (equip_type.contains(result)) {
    return equip_type.at(result);
  }
  return u"";
}

wz::Node *EquipGameInstance::load_equip_info(const std::u16string &id) {
  static std::flat_map<std::u16string, wz::Node *> cache;
  if (!cache.contains(id)) {
    auto equip_type = load_equip_type(id);
    cache[id] =
        WzResource::character->find(equip_type + u"/" + id + u".img/info");
  }
  return cache[id];
}

EquipGameInstance::WeaponType
EquipGameInstance::load_weapon_type(const GameCharacter &g_character) {
  if (!g_character.weapon.has_value()) {
    return WeaponType::NONE;
  }
  auto g_weapon = g_character.weapon->id;
  auto g_weapon_info = EquipGameInstance::load_equip_info(g_weapon);
  auto attack_type =
      static_cast<wz::Property<int16_t> *>(g_weapon_info->get_child(u"attack"))
          ->get();
  return (WeaponType)attack_type;
}

EquipGameInstance::WeaponEnum
EquipGameInstance::load_weapon_enum(const std::u16string &id) {
  if (id.empty()) {
    return WeaponEnum::NONE;
  }
  // 装备 id 形如 "013xxxxx"：大类码 = id / 10000，低两位即武器类别（130 → 30）
  std::string s{id.begin(), id.end()};
  return static_cast<WeaponEnum>((std::stoi(s) / 10000) % 100);
}

std::u16string EquipGameInstance::load_equip_name(const std::u16string &id) {
  static auto str_node = WzResource::string->find(u"Eqp.img/ClassicWorld");
  auto type = load_equip_type(id);

  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });
  std::u16string result(view.begin(), view.end());

  auto str = str_node->get_child(type)->get_child(result)->get_child(u"name");
  return static_cast<wz::Property<std::u16string> *>(str)->get();
}

std::u16string EquipGameInstance::load_equip_desc(const std::u16string &id) {
  static auto str_node = WzResource::string->find(u"Eqp.img/ClassicWorld");
  auto type = load_equip_type(id);

  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });
  std::u16string result(view.begin(), view.end());

  auto str = str_node->get_child(type)->get_child(result)->get_child(u"desc");
  if (str == nullptr) {
    return u"";
  }
  return TextGameInstance::load_rstr(str);
}

std::flat_set<JobType>
EquipGameInstance::load_equip_job(const std::u16string &id) {
  std::flat_set<JobType> r;
  auto equip_info = EquipGameInstance::load_equip_info(id);
  auto reqJob =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqJob"))->get();
  if (reqJob == 0) {
    r = {
        JobType::BEGINNER, JobType::WARRIOR, JobType::MAGICIAN,
        JobType::BOWMAN,   JobType::THIEF,
    };
  } else {
    if (reqJob & 1) {
      r.insert(JobType::WARRIOR);
    }
    if (reqJob & 2) {
      r.insert(JobType::MAGICIAN);
    }
    if (reqJob & 4) {
      r.insert(JobType::MAGICIAN);
    }
    if (reqJob & 8) {
      r.insert(JobType::THIEF);
    }
  }
  return r;
}

std::flat_map<EquipGameInstance::IncType, int>
EquipGameInstance::load_equip_inc(const std::u16string &id) {
  std::flat_map<EquipGameInstance::IncType, int> r;
  auto equip_info = load_equip_info(id);
  static const std::flat_map<std::u16string, IncType> incs = {
      {u"attackSpeed", EquipGameInstance::IncType::WEAPON_SPEED},
      {u"incWAT", EquipGameInstance::IncType::WEAPON_PAD},
      {u"incMMP", EquipGameInstance::IncType::WEAPON_MAD},
      {u"incPDD", EquipGameInstance::IncType::PDD},
      {u"incACC", EquipGameInstance::IncType::ACC},
      {u"incSTR", EquipGameInstance::IncType::STR},
      {u"incDEX", EquipGameInstance::IncType::DEX},
      {u"incINT", EquipGameInstance::IncType::INT},
      {u"incLUK", EquipGameInstance::IncType::LUK},
  };
  for (auto [k, v] : incs) {
    if (equip_info->get_children()->contains(k)) {
      auto node = equip_info->get_child(k);
      r[v] = static_cast<wz::Property<int> *>(node)->get();
    }
  }
  return r;
}

std::flat_map<EquipGameInstance::IncType, int>
EquipGameInstance::load_scroll_inc(const std::u16string &id) {
  std::flat_map<EquipGameInstance::IncType, int> r;
  auto scroll_info = ItemGameInstance::load_item_info(id, 0);
  static const std::flat_map<std::u16string, IncType> incs = {
      {u"incWAT", EquipGameInstance::IncType::WEAPON_PAD},
      {u"incMMP", EquipGameInstance::IncType::WEAPON_MAD},
      {u"incPDD", EquipGameInstance::IncType::PDD},
      {u"incACC", EquipGameInstance::IncType::ACC},
      {u"incSTR", EquipGameInstance::IncType::STR},
      {u"incDEX", EquipGameInstance::IncType::DEX},
      {u"incINT", EquipGameInstance::IncType::INT},
      {u"incLUK", EquipGameInstance::IncType::LUK},
      {u"incCRT", EquipGameInstance::IncType::CRTR},
  };
  for (auto [k, v] : incs) {
    if (scroll_info->get_children()->contains(k)) {
      auto node = scroll_info->get_child(k);
      r[v] = static_cast<wz::Property<int> *>(node)->get();
    }
  }
  return r;
}

uint8_t EquipGameInstance::load_equip_tuc(const std::u16string &id) {
  auto equip_info = load_equip_info(id);
  return static_cast<wz::Property<int> *>(equip_info->get_child(u"tuc"))->get();
}

void EquipGameInstance::add_equip(GameEquipItem &equip,
                                    GameCharacter &character, int slot) {
  auto type = load_equip_type(equip.id);
  if (type == u"Accessory") {
    CharacterGameInstance::add_accessory(character, equip.id);
    character.accessory = equip;
  } else if (type == u"Cap") {
    CharacterGameInstance::add_cap(character, equip.id);
    character.cap = equip;
  } else if (type == u"Cape") {
    CharacterGameInstance::add_cape(character, equip.id);
    character.cape = equip;
  } else if (type == u"Coat") {
    CharacterGameInstance::add_coat(character, equip.id);
    character.coat = equip;
  } else if (type == u"Glove") {
    CharacterGameInstance::add_glove(character, equip.id);
    character.glove = equip;
  } else if (type == u"Longcoat") {
    CharacterGameInstance::add_longcoat(character, equip.id);
    character.longcoat = equip;
  } else if (type == u"Pants") {
    CharacterGameInstance::add_pants(character, equip.id);
    character.pant = equip;
  } else if (type == u"Ring") {
    if (slot == -1) {

    } else {
    }
  } else if (type == u"Shield") {
    CharacterGameInstance::add_shield(character, equip.id);
    character.shield = equip;
  } else if (type == u"Shoes") {
    CharacterGameInstance::add_shoes(character, equip.id);
    character.shoes = equip;
  } else if (type == u"Weapon") {
    CharacterGameInstance::add_weapon(character, equip.id);
    character.weapon = equip;
  }
}

bool EquipGameInstance::add_equip_limit(GameEquipItem &equip,
                                          GameCharacter &character, int slot) {
  auto equip_info = load_equip_info(equip.id);
  auto require_job =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqJob"))->get();
  auto require_lv =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLevel"))
          ->get();
  auto require_str =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqSTR"))->get();
  auto require_dex =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqDEX"))->get();
  auto require_int =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqINT"))->get();
  auto require_luk =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLUK"))->get();

  auto jobs = EquipGameInstance::load_equip_job(equip.id);
  auto job = JobSkillGameInstance::load_ski_tree(character.job);
  bool job_r = false;
  for (auto j : job) {
    if (jobs.contains(j)) {
      job_r = true;
      break;
    }
  }
  if (!job_r) {
    return false;
  }
  auto lv = character.level;
  if (lv < require_lv) {
    return false;
  }
  auto str_ap = CharacterStatGameInstance::get_str_ap();
  if (str_ap < require_str) {
    return false;
  }
  auto dex_ap = CharacterStatGameInstance::get_dex_ap();
  if (dex_ap < require_dex) {
    return false;
  }
  auto int_ap = CharacterStatGameInstance::get_int_ap();
  if (int_ap < require_int) {
    return false;
  }
  auto luk_ap = CharacterStatGameInstance::get_luk_ap();
  if (luk_ap < require_luk) {
    return false;
  }
  add_equip(equip, character, slot);
  return true;
}

bool EquipGameInstance::add_equip_deco(GameDecoItem &deco,
                                         GameCharacter &character, int slot) {
  if (add_equip_deco_limit(deco, character, slot)) {
    auto type = load_equip_type(deco.id);
    if (type == u"Accessory") {
      CharacterGameInstance::add_accessory_deco(character, deco.id);
    } else if (type == u"Cap") {
      CharacterGameInstance::add_cap_deco(character, deco.id);
    } else if (type == u"Cape") {
      CharacterGameInstance::add_cape_deco(character, deco.id);
    } else if (type == u"Coat") {
      CharacterGameInstance::add_coat_deco(character, deco.id);
    } else if (type == u"Glove") {
      CharacterGameInstance::add_glove_deco(character, deco.id);
    } else if (type == u"Longcoat") {
      CharacterGameInstance::add_longcoat_deco(character, deco.id);
    } else if (type == u"Pants") {
      CharacterGameInstance::add_pants_deco(character, deco.id);
    } else if (type == u"Ring") {
      if (slot == -1) {
        if (!character.ring0_deco.has_value()) {
          CharacterGameInstance::add_ring0_deco(character, deco.id);
        } else {
          CharacterGameInstance::add_ring1_deco(character, deco.id);
        }
      }
    } else if (type == u"Shield") {
      CharacterGameInstance::add_shield_deco(character, deco.id);
    } else if (type == u"Shoes") {
      CharacterGameInstance::add_shoes_deco(character, deco.id);
    } else if (type == u"Weapon") {
      CharacterGameInstance::add_weapon_deco(character, deco.id);
    }
    return true;
  }
  return false;
}

bool EquipGameInstance::add_equip_deco_limit(GameDecoItem &deco,
                                               GameCharacter &character,
                                               int slot) {
  auto type = load_equip_type(deco.id);
  if (type == u"Weapon") {
    if (!character.weapon.has_value()) {
      return false;
    }
    auto equip_info = load_equip_info(deco.id);
    std::u16string sub = character.weapon->id.substr(2, 2);
    return equip_info->find(u"../" + sub);
  }
  return true;
}

std::vector<GameEquipItem>
EquipGameInstance::load_equip_slot(const std::u16string &id,
                                     GameCharacter &character) {
  std::vector<GameEquipItem> r;
  auto type = load_equip_type(id);
  if (type == u"Accessory") {
    if (character.accessory.has_value()) {
      r.push_back(character.accessory.value());
    }
  } else if (type == u"Cap") {
    if (character.cap.has_value()) {
      r.push_back(character.cap.value());
    }
  } else if (type == u"Cape") {
    if (character.cape.has_value()) {
      r.push_back(character.cape.value());
    }
  } else if (type == u"Coat") {
    if (character.coat.has_value()) {
      r.push_back(character.coat.value());
    }
  } else if (type == u"Glove") {
    if (character.glove.has_value()) {
      r.push_back(character.glove.value());
    }
  } else if (type == u"Longcoat") {
    if (character.coat.has_value()) {
      r.push_back(character.coat.value());
    }
    if (character.pant.has_value()) {
      r.push_back(character.pant.value());
    }
  } else if (type == u"Pants") {
    if (character.pant.has_value()) {
      r.push_back(character.pant.value());
    }
  } else if (type == u"Ring") {
    if (!character.ring0.has_value())
      return r;
    if (!character.ring1.has_value())
      return r;
    if (!character.ring2.has_value())
      return r;
    if (!character.ring3.has_value())
      return r;
    r.push_back(character.ring0.value());
    return r;
  } else if (type == u"Shield") {
    if (character.shield.has_value()) {
      r.push_back(character.shield.value());
    }
  } else if (type == u"Shoes") {
    if (character.shoes.has_value()) {
      r.push_back(character.shoes.value());
    }
  } else if (type == u"Weapon") {
    if (character.weapon.has_value()) {
      r.push_back(character.weapon.value());
    }
  }
  return r;
}

std::vector<GameDecoItem>
EquipGameInstance::load_deco_slot(const std::u16string &id,
                                    GameCharacter &character) {
  std::vector<GameDecoItem> r;
  auto type = load_equip_type(id);
  if (type == u"Accessory") {
    if (character.accessory_deco.has_value()) {
      r.push_back(character.accessory_deco.value());
    }
  } else if (type == u"Cap") {
    if (character.cap_deco.has_value()) {
      r.push_back(character.cap_deco.value());
    }
  } else if (type == u"Cape") {
    if (character.cape_deco.has_value()) {
      r.push_back(character.cape_deco.value());
    }
  } else if (type == u"Coat") {
    if (character.coat_deco.has_value()) {
      r.push_back(character.coat_deco.value());
    }
  } else if (type == u"Glove") {
    if (character.glove_deco.has_value()) {
      r.push_back(character.glove_deco.value());
    }
  } else if (type == u"Longcoat") {
    if (character.coat_deco.has_value()) {
      r.push_back(character.coat_deco.value());
    }
    if (character.pant_deco.has_value()) {
      r.push_back(character.pant_deco.value());
    }
  } else if (type == u"Pants") {
    if (character.pant_deco.has_value()) {
      r.push_back(character.pant_deco.value());
    }
  } else if (type == u"Ring") {
    if (!character.ring0_deco.has_value())
      return r;
    if (!character.ring1_deco.has_value())
      return r;
    r.push_back(character.ring1_deco.value());
    return r;
  } else if (type == u"Shield") {
    if (character.shield_deco.has_value()) {
      r.push_back(character.shield_deco.value());
    }
  } else if (type == u"Shoes") {
    if (character.shoes_deco.has_value()) {
      r.push_back(character.shoes_deco.value());
    }
  } else if (type == u"Weapon") {
    if (character.weapon_deco.has_value()) {
      r.push_back(character.weapon_deco.value());
    }
  }
  return r;
}

std::vector<GameEquipItem>
EquipGameInstance::load_equips(const GameCharacter &c) {
  std::vector<GameEquipItem> v;
  if (c.cap.has_value()) {
    v.push_back(c.cap.value());
  }
  if (c.cape.has_value()) {
    v.push_back(c.cape.value());
  }
  if (c.coat.has_value()) {
    v.push_back(c.coat.value());
  }
  if (c.longcoat.has_value()) {
    v.push_back(c.longcoat.value());
  }
  if (c.weapon.has_value()) {
    v.push_back(c.weapon.value());
  }
  if (c.pant.has_value()) {
    v.push_back(c.pant.value());
  }
  if (c.shield.has_value()) {
    v.push_back(c.shield.value());
  }
  if (c.shoes.has_value()) {
    v.push_back(c.shoes.value());
  }
  if (c.ring0.has_value()) {
    v.push_back(c.ring0.value());
  }
  if (c.ring1.has_value()) {
    v.push_back(c.ring1.value());
  }
  if (c.ring2.has_value()) {
    v.push_back(c.ring2.value());
  }
  if (c.ring3.has_value()) {
    v.push_back(c.ring3.value());
  }
  return v;
}

std::vector<GameDecoItem>
EquipGameInstance::load_decos(const GameCharacter &c) {
  std::vector<GameDecoItem> v;
  if (c.coat_deco.has_value()) {
    v.push_back(c.coat_deco.value());
  }
  if (c.cap_deco.has_value()) {
    v.push_back(c.cap_deco.value());
  }
  if (c.pant_deco.has_value()) {
    v.push_back(c.pant_deco.value());
  }
  if (c.shoes_deco.has_value()) {
    v.push_back(c.shoes_deco.value());
  }
  if (c.shield_deco.has_value()) {
    v.push_back(c.shield_deco.value());
  }
  if (c.cape_deco.has_value()) {
    v.push_back(c.cape_deco.value());
  }
  if (c.accessory_deco.has_value()) {
    v.push_back(c.accessory_deco.value());
  }
  if (c.glove_deco.has_value()) {
    v.push_back(c.glove_deco.value());
  }
  if (c.longcoat_deco.has_value()) {
    v.push_back(c.longcoat_deco.value());
  }
  if (c.weapon_deco.has_value()) {
    v.push_back(c.weapon_deco.value());
  }
  if (c.ring0_deco.has_value()) {
    v.push_back(c.ring0_deco.value());
  }
  if (c.ring1_deco.has_value()) {
    v.push_back(c.ring1_deco.value());
  }
  return v;
}

int EquipGameInstance::use_equip_scroll(GameEquipItem &eqp,
                                          GameConsumeItem &s) {
  auto tuc = EquipGameInstance::load_equip_tuc(eqp.id);
  if (eqp.scroll.size() < tuc) {
    auto item_info = ItemGameInstance::load_item_info(s.id, 0);
    int success = 100;
    if (item_info->get_child(u"success")) {
      success =
          static_cast<wz::Property<int> *>(item_info->get_child(u"success"))
              ->get();
    }
    int cursed = 0;
    if (item_info->get_child(u"cursed")) {
      cursed = static_cast<wz::Property<int> *>(item_info->get_child(u"cursed"))
                   ->get();
    }
    eqp.scroll.push_back({s.id, true});

    auto &sf = CharacterGameInstance::self;
    ServerCharacterInstance::handle_scroll_use(sf, true);

    StateT st;
    st.state = fbs::StateEnum_ITEM_USE;
    st.val = std::stoi(std::string{s.id.begin(), s.id.end()});
    st.sub_val = 1;
    CharacterLogicSystem::ccs.payload.push_back(std::make_unique<StateT>(st));

    s.num -= 1;
    if (s.num == 0) {
      s.id = u"";
    }
  }
  CharacterStatGameInstance::update();
  return false;
}

int EquipGameInstance::use_equip(int i, int slot) {
  auto &equips = PackageGameInstance::data[(int)ItemEnum::equip];
  auto &itm = equips[i];
  auto &sf = CharacterGameInstance::self;
  auto &eq = static_cast<GameEquipItem &>(*itm);
  auto ev = EquipGameInstance::load_equip_slot(eq.id, sf);
  auto blank_slot =
      PackageGameInstance::load_empty_index((int)ItemEnum::equip);
  blank_slot.push_back(i);
  std::ranges::sort(blank_slot);
  if (blank_slot.size() < ev.size()) {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::equip_no_space;
    NoticeUiSystem::open();
    return 0;
  }
  if (!EquipGameInstance::add_equip_limit(eq, sf, slot)) {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::equip_no_ability;
    NoticeUiSystem::open();
    return 0;
  }
  for (int32_t i = 0; i < ev.size(); i++) {
    equips[blank_slot[i]] = std::polymorphic<GameItem>(ev[i]);
  }
  itm->id = u"";
  CharacterLogicSystem::cct.map_id = SceneSystemInstance::map_id;
  CharacterStatGameInstance::update();
  return 1;
}

int EquipGameInstance::use_deco(int i, int slot) {
  auto &equips = PackageGameInstance::data[(int)ItemEnum::deco];
  auto &itm = equips[i];
  auto &sf = CharacterGameInstance::self;
  auto &de = static_cast<GameDecoItem &>(*itm);
  auto ev = EquipGameInstance::load_deco_slot(de.id, sf);
  auto blank_slot =
      PackageGameInstance::load_empty_index((int)ItemEnum::equip);
  blank_slot.push_back(i);
  std::ranges::sort(blank_slot);
  if (blank_slot.size() < ev.size()) {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::equip_no_space;
    NoticeUiSystem::open();
    return 0;
  }
  if (!EquipGameInstance::add_equip_deco(de, sf, 0)) {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::equip_no_ability;
    NoticeUiSystem::open();
    return 0;
  }
  for (int32_t i = 0; i < ev.size(); i++) {
    equips[blank_slot[i]] = std::polymorphic<GameItem>(ev[i]);
  }
  itm->id = u"";
  CharacterLogicSystem::cct.map_id = SceneSystemInstance::map_id;
  return 1;
}

std::optional<GameEquipItem> *EquipGameInstance::load_equip(int index) {
  std::optional<GameEquipItem> *equip;
  auto &self = CharacterGameInstance::self;
  switch ((EquipMouseIndex)index) {
  case EquipMouseIndex::cap: {
    equip = &self.cap;
    break;
  }
  case EquipMouseIndex::earcc: {
    equip = &self.accessory;
    break;
  }
  case EquipMouseIndex::clothes: {
    equip = &self.coat;
    break;
  }
  case EquipMouseIndex::pants: {
    equip = &self.pant;
    break;
  }
  case EquipMouseIndex::shoes: {
    equip = &self.shoes;
    break;
  }
  case EquipMouseIndex::gloves: {
    equip = &self.glove;
    break;
  }
  case EquipMouseIndex::cape: {
    equip = &self.cape;
    break;
  }
  case EquipMouseIndex::shield: {
    equip = &self.shield;
    break;
  }
  case EquipMouseIndex::weapon: {
    equip = &self.weapon;
    break;
  }
  case EquipMouseIndex::ring0:
  case EquipMouseIndex::ring1:
  case EquipMouseIndex::ring2:
  case EquipMouseIndex::ring3:
    break;
  }
  return equip;
}

std::optional<GameDecoItem> *EquipGameInstance::load_deco(int index) {
  std::optional<GameDecoItem> *deco;
  auto &self = CharacterGameInstance::self;
  switch ((EquipMouseIndex)index) {
  case EquipMouseIndex::cap: {
    deco = &self.cap_deco;
    break;
  }
  case EquipMouseIndex::earcc: {
    deco = &self.accessory_deco;
    break;
  }
  case EquipMouseIndex::clothes: {
    deco = &self.coat_deco;
    break;
  }
  case EquipMouseIndex::pants: {
    deco = &self.pant_deco;
    break;
  }
  case EquipMouseIndex::shoes: {
    deco = &self.shoes_deco;
    break;
  }
  case EquipMouseIndex::gloves: {
    deco = &self.glove_deco;
    break;
  }
  case EquipMouseIndex::cape: {
    deco = &self.cape_deco;
    break;
  }
  case EquipMouseIndex::shield: {
    deco = &self.shield_deco;
    break;
  }
  case EquipMouseIndex::weapon: {
    deco = &self.weapon_deco;
    break;
  }
  case EquipMouseIndex::ring0:
  case EquipMouseIndex::ring1:
  case EquipMouseIndex::ring2:
  case EquipMouseIndex::ring3:
    break;
  }
  return deco;
}

int EquipGameInstance::unuse_equip(int i) {
  auto eqp = std::polymorphic<GameItem>(load_equip(i)->value());
  if (!PackageGameInstance::add_item(eqp)) {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::equip_no_space;
    NoticeUiSystem::open();
    return 0;
  }
  (*load_equip(i)) = std::nullopt;
  CharacterLogicSystem::cct.map_id = SceneSystemInstance::map_id;
  CharacterStatGameInstance::update();
  return 1;
}

int EquipGameInstance::unuse_deco(int i) {
  auto eqp = std::polymorphic<GameItem>(load_deco(i)->value());
  if (!PackageGameInstance::add_item(eqp)) {
    NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::equip_no_space;
    NoticeUiSystem::open();
    return 0;
  }
  (*load_deco(i)) = std::nullopt;
  CharacterLogicSystem::cct.map_id = SceneSystemInstance::map_id;
  return 1;
}
