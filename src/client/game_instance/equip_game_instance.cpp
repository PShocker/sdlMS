#include "src/client/game_instance/equip_game_instance.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/cursor_logic_system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"

#include <wz/Property.h>

#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <ranges>
#include <string>
#include <vector>


namespace sdlms {


namespace {

bool IsPetEquipItem(int item_id) {
  return item_id / 100000 == 18;
}

bool IsBlackUpgradeItem(int item_id) {
  return item_id / 100 == 20491;
}

bool IsAccUpgradeItem(int item_id) {
  return item_id / 100 == 20492;
}

bool IsHyperUpgradeItem(int item_id) {
  return item_id / 100 == 20493;
}

bool IsItemOptionUpgradeItem(int item_id) {
  return item_id / 100 == 20494;
}

bool IsNewUpgradeItem(int item_id) {
  return item_id / 1000 == 2046;
}

bool IsDurabilityUpgradeItem(int item_id) {
  return item_id / 1000 == 2047;
}


// 095 is_correct_upgrade_equip: scroll id -> applicable equip category.

bool IsCorrectUpgradeEquip(int scroll_id, int equip_id) {
  const int scroll_type = scroll_id / 10000;

  if (scroll_type == 249 || scroll_type == 247) {
    return true;
  }

  if (scroll_type != 204 || equip_id / 1000000 != 1) {
    return false;
  }

  if (((scroll_id / 100 == 20490 || IsBlackUpgradeItem(scroll_id)) &&

       !IsPetEquipItem(equip_id)) ||

      IsHyperUpgradeItem(scroll_id) ||

      IsItemOptionUpgradeItem(scroll_id)) {
    return true;
  }

  const int scroll_category = (scroll_id - 2040000) / 100;

  const int equip_category = equip_id / 10000 % 100;

  if (IsAccUpgradeItem(scroll_id)) {
    return equip_category >= 11 && equip_category <= 13;
  }

  if (IsNewUpgradeItem(scroll_id) || IsDurabilityUpgradeItem(scroll_id)) {
    switch (scroll_category % 10) {
      case 0:

        return equip_category >= 30 && equip_category <= 39;

      case 1:

        return equip_category >= 40 && equip_category <= 49;

      case 2:

        return equip_category == 0 ||

               (equip_category > 3 && equip_category <= 10);

      case 3:

        switch (equip_category) {
          case 1:

          case 2:

          case 3:

          case 11:

          case 12:

          case 13:

          case 14:

            return true;

          default:

            return false;
        }

      default:

        return true;
    }
  }

  return scroll_category == equip_category;
}

}  // namespace


bool EquipGameInstance::CheckEquip(const std::u16string& id) {
  auto r = id.substr(0, 2);

  if (r == u"01") {
    return true;
  }

  return false;
}


std::polymorphic<GameItem> EquipGameInstance::LoadItem(

    const std::u16string& id) {
  GameEquipItem eqp;

  eqp.type = ItemEnum::kEquip;

  eqp.id = id;

  auto r = std::polymorphic<GameItem>(eqp);

  return r;
}


std::u16string EquipGameInstance::LoadEquipType(const std::u16string& id) {
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
    const auto& fallback = equip_type.at(result);

    return WzResource::LoadMSString(u"equipType" + fallback, fallback);
  }

  return u"";
}


wz::Node* EquipGameInstance::LoadEquipInfo(const std::u16string& id) {
  static std::flat_map<std::u16string, wz::Node*> cache;

  if (!cache.contains(id)) {
    auto equip_type = LoadEquipType(id);

    if (equip_type.empty()) {
      cache[id] = nullptr;

    } else {
      cache[id] =

          WzResource::character_->find(equip_type + u"/" + id + u".img/info");
    }
  }

  return cache[id];
}


EquipGameInstance::WeaponType EquipGameInstance::LoadWeaponType(

    const GameCharacter& g_character) {
  if (!g_character.weapon.has_value()) {
    return WeaponType::kNone;
  }

  auto g_weapon = g_character.weapon->id;

  auto g_weapon_info = EquipGameInstance::LoadEquipInfo(g_weapon);

  auto attack_type =

      static_cast<wz::Property<int16_t>*>(g_weapon_info->get_child(u"attack"))

          ->get();

  return (WeaponType)attack_type;
}


EquipGameInstance::WeaponEnum EquipGameInstance::LoadWeaponEnum(

    const std::u16string& id) {
  if (id.empty()) {
    return WeaponEnum::kNone;
  }

  // 装备 id 形如 "013xxxxx"：大类码 = id / 10000，低两位即武器类别（130 �?30�?
  std::string s{id.begin(), id.end()};

  return static_cast<WeaponEnum>((std::stoi(s) / 10000) % 100);
}


std::u16string EquipGameInstance::LoadEquipName(const std::u16string& id) {
  static auto str_node = WzResource::string_->find(u"Eqp.img/ClassicWorld");

  auto type = LoadEquipType(id);


  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });

  std::u16string result(view.begin(), view.end());


  auto str = str_node->get_child(type)->get_child(result)->get_child(u"name");

  return static_cast<wz::Property<std::u16string>*>(str)->get();
}


std::u16string EquipGameInstance::LoadEquipDesc(const std::u16string& id) {
  static auto str_node = WzResource::string_->find(u"Eqp.img/ClassicWorld");

  auto type = LoadEquipType(id);


  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });

  std::u16string result(view.begin(), view.end());


  auto str = str_node->get_child(type)->get_child(result)->get_child(u"desc");

  if (str == nullptr) {
    return u"";
  }

  return TextGameInstance::LoadRstr(str);
}


std::flat_set<JobType> EquipGameInstance::LoadEquipJob(

    const std::u16string& id) {
  std::flat_set<JobType> r;

  auto equip_info = EquipGameInstance::LoadEquipInfo(id);

  auto req_job =

      static_cast<wz::Property<int>*>(equip_info->get_child(u"reqJob"))->get();

  if (req_job == 0) {
    r = {

        JobType::kBeginner, JobType::kWarrior, JobType::kMagician,

        JobType::kBowman,   JobType::kThief,

    };

  } else {
    if (req_job & 1) {
      r.insert(JobType::kWarrior);
    }

    if (req_job & 2) {
      r.insert(JobType::kMagician);
    }

    if (req_job & 4) {
      r.insert(JobType::kMagician);
    }

    if (req_job & 8) {
      r.insert(JobType::kThief);
    }
  }

  return r;
}


std::flat_map<EquipGameInstance::IncType, int> EquipGameInstance::LoadEquipInc(

    const std::u16string& id) {
  std::flat_map<EquipGameInstance::IncType, int> r;

  auto equip_info = LoadEquipInfo(id);

  static const std::flat_map<std::u16string, IncType> incs = {

      {u"attackSpeed", EquipGameInstance::IncType::kWeaponSpeed},

      {u"incWAT", EquipGameInstance::IncType::kWeaponPad},

      {u"incMMP", EquipGameInstance::IncType::kWeaponMad},

      {u"incPDD", EquipGameInstance::IncType::kPdd},

      {u"incACC", EquipGameInstance::IncType::kAcc},

      {u"incSTR", EquipGameInstance::IncType::kStr},

      {u"incDEX", EquipGameInstance::IncType::kDex},

      {u"incINT", EquipGameInstance::IncType::kInt},

      {u"incLUK", EquipGameInstance::IncType::kLuk},

  };

  for (auto [k, v] : incs) {
    if (equip_info->get_children()->contains(k)) {
      auto node = equip_info->get_child(k);

      r[v] = static_cast<wz::Property<int>*>(node)->get();
    }
  }

  return r;
}


std::flat_map<EquipGameInstance::IncType, int> EquipGameInstance::LoadScrollInc(

    const std::u16string& id) {
  std::flat_map<EquipGameInstance::IncType, int> r;

  auto scroll_info = ItemGameInstance::LoadItemInfo(id, 0);

  static const std::flat_map<std::u16string, IncType> incs = {

      {u"incWAT", EquipGameInstance::IncType::kWeaponPad},

      {u"incMMP", EquipGameInstance::IncType::kWeaponMad},

      {u"incPDD", EquipGameInstance::IncType::kPdd},

      {u"incACC", EquipGameInstance::IncType::kAcc},

      {u"incSTR", EquipGameInstance::IncType::kStr},

      {u"incDEX", EquipGameInstance::IncType::kDex},

      {u"incINT", EquipGameInstance::IncType::kInt},

      {u"incLUK", EquipGameInstance::IncType::kLuk},

      {u"incCRT", EquipGameInstance::IncType::kCrtr},

  };

  for (auto [k, v] : incs) {
    if (scroll_info->get_children()->contains(k)) {
      auto node = scroll_info->get_child(k);

      r[v] = static_cast<wz::Property<int>*>(node)->get();
    }
  }

  return r;
}


uint8_t EquipGameInstance::LoadEquipTuc(const std::u16string& id) {
  auto equip_info = LoadEquipInfo(id);

  return static_cast<wz::Property<int>*>(equip_info->get_child(u"tuc"))->get();
}


void EquipGameInstance::AddEquip(GameEquipItem& equip, GameCharacter& character,

                                 int slot) {
  auto type = LoadEquipType(equip.id);

  if (type == u"Accessory") {
    CharacterGameInstance::AddAccessory(character, equip.id);

    character.accessory = equip;

  } else if (type == u"Cap") {
    CharacterGameInstance::AddCap(character, equip.id);

    character.cap = equip;

  } else if (type == u"Cape") {
    CharacterGameInstance::AddCape(character, equip.id);

    character.cape = equip;

  } else if (type == u"Coat") {
    // 上衣和套服互斥；裤子保留。
    character.longcoat.reset();
    CharacterGameInstance::AddCoat(character, equip.id);

    character.coat = equip;

  } else if (type == u"Glove") {
    CharacterGameInstance::AddGlove(character, equip.id);

    character.glove = equip;

  } else if (type == u"Longcoat") {
    // 套服占用上衣 + 裤子两个槽位。
    character.coat.reset();
    character.pant.reset();
    CharacterGameInstance::AddLongcoat(character, equip.id);

    character.longcoat = equip;

  } else if (type == u"Pants") {
    // 裤子和套服互斥；上衣保留。
    character.longcoat.reset();
    CharacterGameInstance::AddPants(character, equip.id);

    character.pant = equip;

  } else if (type == u"Ring") {
    if (slot == -1) {
    } else {
    }

  } else if (type == u"Shield") {
    CharacterGameInstance::AddShield(character, equip.id);

    character.shield = equip;

  } else if (type == u"Shoes") {
    CharacterGameInstance::AddShoes(character, equip.id);

    character.shoes = equip;

  } else if (type == u"Weapon") {
    CharacterGameInstance::AddWeapon(character, equip.id);

    character.weapon = equip;
  }
}


bool EquipGameInstance::AddEquipLimit(GameEquipItem& equip,

                                      GameCharacter& character, int slot) {
  auto equip_info = LoadEquipInfo(equip.id);

  auto require_job =

      static_cast<wz::Property<int>*>(equip_info->get_child(u"reqJob"))->get();

  auto require_lv =

      static_cast<wz::Property<int>*>(equip_info->get_child(u"reqLevel"))

          ->get();

  auto require_str =

      static_cast<wz::Property<int>*>(equip_info->get_child(u"reqSTR"))->get();

  auto require_dex =

      static_cast<wz::Property<int>*>(equip_info->get_child(u"reqDEX"))->get();

  auto require_int =

      static_cast<wz::Property<int>*>(equip_info->get_child(u"reqINT"))->get();

  auto require_luk =

      static_cast<wz::Property<int>*>(equip_info->get_child(u"reqLUK"))->get();


  auto jobs = EquipGameInstance::LoadEquipJob(equip.id);

  auto job = JobSkillGameInstance::LoadSkiTree(character.job);

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

  auto str_ap = CharacterStatGameInstance::GetStrAp();

  if (str_ap < require_str) {
    return false;
  }

  auto dex_ap = CharacterStatGameInstance::GetDexAp();

  if (dex_ap < require_dex) {
    return false;
  }

  auto int_ap = CharacterStatGameInstance::GetIntAp();

  if (int_ap < require_int) {
    return false;
  }

  auto luk_ap = CharacterStatGameInstance::GetLukAp();

  if (luk_ap < require_luk) {
    return false;
  }

  AddEquip(equip, character, slot);

  return true;
}


bool EquipGameInstance::AddEquipDeco(GameDecoItem& deco,

                                     GameCharacter& character, int slot) {
  if (AddEquipDecoLimit(deco, character, slot)) {
    auto type = LoadEquipType(deco.id);

    if (type == u"Accessory") {
      CharacterGameInstance::AddAccessoryDeco(character, deco.id);

    } else if (type == u"Cap") {
      CharacterGameInstance::AddCapDeco(character, deco.id);

    } else if (type == u"Cape") {
      CharacterGameInstance::AddCapeDeco(character, deco.id);

    } else if (type == u"Coat") {
      CharacterGameInstance::AddCoatDeco(character, deco.id);

    } else if (type == u"Glove") {
      CharacterGameInstance::AddGloveDeco(character, deco.id);

    } else if (type == u"Longcoat") {
      CharacterGameInstance::AddLongcoatDeco(character, deco.id);

    } else if (type == u"Pants") {
      CharacterGameInstance::AddPantsDeco(character, deco.id);

    } else if (type == u"Ring") {
      if (slot == -1) {
        if (!character.ring0_deco.has_value()) {
          CharacterGameInstance::AddRing0Deco(character, deco.id);

        } else {
          CharacterGameInstance::AddRing1Deco(character, deco.id);
        }
      }

    } else if (type == u"Shield") {
      CharacterGameInstance::AddShieldDeco(character, deco.id);

    } else if (type == u"Shoes") {
      CharacterGameInstance::AddShoesDeco(character, deco.id);

    } else if (type == u"Weapon") {
      CharacterGameInstance::AddWeaponDeco(character, deco.id);
    }

    return true;
  }

  return false;
}


bool EquipGameInstance::AddEquipDecoLimit(GameDecoItem& deco,

                                          GameCharacter& character, int slot) {
  auto type = LoadEquipType(deco.id);

  if (type == u"Weapon") {
    if (!character.weapon.has_value()) {
      return false;
    }

    auto equip_info = LoadEquipInfo(deco.id);

    std::u16string sub = character.weapon->id.substr(2, 2);

    return equip_info->find(u"../" + sub);
  }

  return true;
}


std::vector<GameEquipItem> EquipGameInstance::LoadEquipSlot(

    const std::u16string& id, GameCharacter& character) {
  std::vector<GameEquipItem> r;

  auto type = LoadEquipType(id);

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
    // 上衣与套服共用 Clothes 槽：穿上衣时套服也要一起脱下。
    if (character.coat.has_value()) {
      r.push_back(character.coat.value());
    }
    if (character.longcoat.has_value()) {
      r.push_back(character.longcoat.value());
    }

  } else if (type == u"Glove") {
    if (character.glove.has_value()) {
      r.push_back(character.glove.value());
    }

  } else if (type == u"Longcoat") {
    // 套服占用上衣 + 裤子两个槽位。
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
    if (character.longcoat.has_value()) {
      r.push_back(character.longcoat.value());
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


std::vector<GameDecoItem> EquipGameInstance::LoadDecoSlot(

    const std::u16string& id, GameCharacter& character) {
  std::vector<GameDecoItem> r;

  auto type = LoadEquipType(id);

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


std::vector<GameEquipItem> EquipGameInstance::LoadEquips(

    const GameCharacter& c) {
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


std::vector<GameDecoItem> EquipGameInstance::LoadDecos(const GameCharacter& c) {
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


int EquipGameInstance::UseEquipScroll(GameEquipItem& eqp, GameConsumeItem& s) {
  int scroll_id = 0;

  int equip_id = 0;

  try {
    scroll_id = std::stoi(std::string{s.id.begin(), s.id.end()});

    equip_id = std::stoi(std::string{eqp.id.begin(), eqp.id.end()});

  } catch (...) {
    CursorGameInstance::cursor_hand_ = std::nullopt;

    return false;
  }

  if (!IsCorrectUpgradeEquip(scroll_id, equip_id)) {
    // 卷轴和装备类型不匹配：清空鼠标手上的卷轴�?
    CursorGameInstance::cursor_hand_ = std::nullopt;

    return false;
  }


  auto tuc = EquipGameInstance::LoadEquipTuc(eqp.id);

  if (eqp.scroll.size() < tuc) {
    auto item_info = ItemGameInstance::LoadItemInfo(s.id, 0);

    int success = 100;

    if (item_info->get_child(u"success")) {
      success =

          static_cast<wz::Property<int>*>(item_info->get_child(u"success"))

              ->get();
    }

    int cursed = 0;

    if (item_info->get_child(u"cursed")) {
      cursed = static_cast<wz::Property<int>*>(item_info->get_child(u"cursed"))

                   ->get();
    }

    std::uniform_int_distribution<int> dist(0, 99);

    const bool scroll_success = dist(RandomGameInstance::gen_) < success;

    eqp.scroll.push_back({s.id, scroll_success});


    auto& sf = CharacterGameInstance::self_;

    ServerCharacterInstance::HandleScrollUse(sf, scroll_success);


    AudioGameInstance::PlayNow(scroll_success ? u"Game.img/EnchantSuccess"

                                              : u"Game.img/EnchantFailure");


    fbs::StateT st;

    st.state = fbs::StateEnum_ITEM_USE;

    st.val = scroll_id;

    st.sub_val = 1;

    CharacterLogicSystem::ccs_.payload.push_back(

        std::make_unique<fbs::StateT>(st));


    s.num -= 1;

    if (s.num == 0) {
      s.id = u"";
    }
  }

  CharacterStatGameInstance::Update();

  return false;
}


int EquipGameInstance::UseEquip(int i, int slot) {
  if (CharacterLogicSystem::IsAlert()) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kCannotChangeEquip;
    NoticeUiSystem::Open();
    return 0;
  }
  auto& equips = PackageGameInstance::data_[(int)ItemEnum::kEquip];

  auto& itm = equips[i];

  auto& sf = CharacterGameInstance::self_;

  auto& eq = static_cast<GameEquipItem&>(*itm);

  auto ev = EquipGameInstance::LoadEquipSlot(eq.id, sf);

  auto blank_slot = PackageGameInstance::LoadEmptyIndex((int)ItemEnum::kEquip);

  blank_slot.push_back(i);

  std::ranges::sort(blank_slot);

  if (blank_slot.size() < ev.size()) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kEquipNoSpace;

    NoticeUiSystem::Open();

    return 0;
  }

  if (!EquipGameInstance::AddEquipLimit(eq, sf, slot)) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kEquipNoAbility;

    NoticeUiSystem::Open();

    return 0;
  }

  // 先清空当前装备槽，再把被替换下来的装备放回背包。
  // 如果 blank_slot 里正好包含当前槽 i，原来的顺序会先写入旧装备，
  // 随后 itm->id = "" 又把旧装备清掉。
  itm->id = u"";

  for (int32_t i = 0; i < ev.size(); i++) {
    equips[blank_slot[i]] = std::polymorphic<GameItem>(ev[i]);
  }

  CharacterLogicSystem::cct_.map_id = SceneSystemInstance::map_id_;

  CharacterStatGameInstance::Update();

  return 1;
}


int EquipGameInstance::UseDeco(int i, int slot) {
  if (CharacterLogicSystem::IsAlert()) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kCannotChangeEquip;
    NoticeUiSystem::Open();
    return 0;
  }
  auto& equips = PackageGameInstance::data_[(int)ItemEnum::kDeco];

  auto& itm = equips[i];

  auto& sf = CharacterGameInstance::self_;

  auto& de = static_cast<GameDecoItem&>(*itm);

  auto ev = EquipGameInstance::LoadDecoSlot(de.id, sf);

  auto blank_slot = PackageGameInstance::LoadEmptyIndex((int)ItemEnum::kDeco);

  blank_slot.push_back(i);

  std::ranges::sort(blank_slot);

  if (blank_slot.size() < ev.size()) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kEquipNoSpace;

    NoticeUiSystem::Open();

    return 0;
  }

  if (!EquipGameInstance::AddEquipDeco(de, sf, 0)) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kEquipNoAbility;

    NoticeUiSystem::Open();

    return 0;
  }

  // 先清空当前装饰槽，再放回被替换的装饰装备。
  itm->id = u"";

  for (int32_t i = 0; i < ev.size(); i++) {
    equips[blank_slot[i]] = std::polymorphic<GameItem>(ev[i]);
  }

  CharacterLogicSystem::cct_.map_id = SceneSystemInstance::map_id_;

  return 1;
}


std::optional<GameEquipItem>* EquipGameInstance::LoadEquip(int index) {
  std::optional<GameEquipItem>* equip;

  auto& self = CharacterGameInstance::self_;

  switch ((EquipMouseIndex)index) {
    case EquipMouseIndex::kCap: {
      equip = &self.cap;

      break;
    }

    case EquipMouseIndex::kEarcc: {
      equip = &self.accessory;

      break;
    }

    case EquipMouseIndex::kClothes: {
      if (self.coat.has_value()) {
        equip = &self.coat;
      } else {
        equip = &self.longcoat;
      }

      break;
    }

    case EquipMouseIndex::kPants: {
      equip = &self.pant;

      break;
    }

    case EquipMouseIndex::kShoes: {
      equip = &self.shoes;

      break;
    }

    case EquipMouseIndex::kGloves: {
      equip = &self.glove;

      break;
    }

    case EquipMouseIndex::kCape: {
      equip = &self.cape;

      break;
    }

    case EquipMouseIndex::kShield: {
      equip = &self.shield;

      break;
    }

    case EquipMouseIndex::kWeapon: {
      equip = &self.weapon;

      break;
    }

    case EquipMouseIndex::kRing0:

    case EquipMouseIndex::kRing1:

    case EquipMouseIndex::kRing2:

    case EquipMouseIndex::kRing3:

      break;
  }

  return equip;
}


std::optional<GameDecoItem>* EquipGameInstance::LoadDeco(int index) {
  std::optional<GameDecoItem>* deco;

  auto& self = CharacterGameInstance::self_;

  switch ((EquipMouseIndex)index) {
    case EquipMouseIndex::kCap: {
      deco = &self.cap_deco;

      break;
    }

    case EquipMouseIndex::kEarcc: {
      deco = &self.accessory_deco;

      break;
    }

    case EquipMouseIndex::kClothes: {
      deco = &self.coat_deco;

      break;
    }

    case EquipMouseIndex::kPants: {
      deco = &self.pant_deco;

      break;
    }

    case EquipMouseIndex::kShoes: {
      deco = &self.shoes_deco;

      break;
    }

    case EquipMouseIndex::kGloves: {
      deco = &self.glove_deco;

      break;
    }

    case EquipMouseIndex::kCape: {
      deco = &self.cape_deco;

      break;
    }

    case EquipMouseIndex::kShield: {
      deco = &self.shield_deco;

      break;
    }

    case EquipMouseIndex::kWeapon: {
      deco = &self.weapon_deco;

      break;
    }

    case EquipMouseIndex::kRing0:

    case EquipMouseIndex::kRing1:

    case EquipMouseIndex::kRing2:

    case EquipMouseIndex::kRing3:

      break;
  }

  return deco;
}


int EquipGameInstance::UnuseEquip(int i) {
  if (CharacterLogicSystem::IsAlert()) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kCannotChangeEquip;
    NoticeUiSystem::Open();
    return 0;
  }
  auto eqp = std::polymorphic<GameItem>(LoadEquip(i)->value());

  if (!PackageGameInstance::AddItem(eqp)) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kEquipNoSpace;

    NoticeUiSystem::Open();

    return 0;
  }

  (*LoadEquip(i)) = std::nullopt;

  CharacterLogicSystem::cct_.map_id = SceneSystemInstance::map_id_;

  CharacterStatGameInstance::Update();

  return 1;
}


int EquipGameInstance::UnuseDeco(int i) {
  if (CharacterLogicSystem::IsAlert()) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kCannotChangeEquip;
    NoticeUiSystem::Open();
    return 0;
  }
  auto eqp = std::polymorphic<GameItem>(LoadDeco(i)->value());

  if (!PackageGameInstance::AddItem(eqp)) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kEquipNoSpace;

    NoticeUiSystem::Open();

    return 0;
  }

  (*LoadDeco(i)) = std::nullopt;

  CharacterLogicSystem::cct_.map_id = SceneSystemInstance::map_id_;

  return 1;
}


}  // namespace sdlms
