#include "equip_game_instance.h"
#include "character_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <ranges>
#include <string>
#include <vector>

std::u16string equip_game_instance::load_equip_type(const std::u16string &id) {
  const auto result = id.substr(1, 3);
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
      {u"170", u"Weapon"}};
  return equip_type.at(result);
}

wz::Node *equip_game_instance::load_equip_info(const std::u16string &id) {
  static std::flat_map<std::u16string, wz::Node *> cache;
  if (!cache.contains(id)) {
    auto equip_type = load_equip_type(id);
    cache[id] =
        wz_resource::character->find(equip_type + u"/" + id + u".img/info");
  }
  return cache[id];
}

equip_game_instance::weapon_type
equip_game_instance::load_weapon_type(const game_character &g_character) {
  if (!g_character.weapon.has_value()) {
    return weapon_type::NONE;
  }
  auto g_weapon = g_character.weapon->id;
  auto g_weapon_info = equip_game_instance::load_equip_info(g_weapon);
  auto attack_type =
      static_cast<wz::Property<int16_t> *>(g_weapon_info->get_child(u"attack"))
          ->get();
  return (weapon_type)attack_type;
}

std::u16string equip_game_instance::load_equip_name(const std::u16string &id) {
  static auto str_node = wz_resource::string->find(u"Eqp.img/ClassicWorld");
  auto type = load_equip_type(id);

  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });
  std::u16string result(view.begin(), view.end());

  auto str = str_node->get_child(type)->get_child(result)->get_child(u"name");
  return static_cast<wz::Property<std::u16string> *>(str)->get();
}

std::flat_set<equip_game_instance::job_type>
equip_game_instance::load_equip_job(const std::u16string &id) {
  std::flat_set<equip_game_instance::job_type> r;
  auto equip_info = equip_game_instance::load_equip_info(id);
  auto reqJob =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqJob"))->get();
  if (reqJob == 0) {
    r = {
        equip_game_instance::job_type::BEGINNER,
        equip_game_instance::job_type::WARRIOR,
        equip_game_instance::job_type::MAGICIAN,
        equip_game_instance::job_type::BOWMAN,
        equip_game_instance::job_type::THIEF,
    };
  } else {
    if (reqJob & 1) {
      r.insert(equip_game_instance::job_type::WARRIOR);
    }
    if (reqJob & 2) {
      r.insert(equip_game_instance::job_type::MAGICIAN);
    }
    if (reqJob & 4) {
      r.insert(equip_game_instance::job_type::MAGICIAN);
    }
    if (reqJob & 8) {
      r.insert(equip_game_instance::job_type::THIEF);
    }
  }
  return r;
}

std::flat_map<equip_game_instance::inc_type, int>
equip_game_instance::load_equip_inc(const std::u16string &id) {
  std::flat_map<equip_game_instance::inc_type, int> r;
  auto equip_info = load_equip_info(id);
  static const std::flat_map<std::u16string, inc_type> incs = {
      {u"attackSpeed", equip_game_instance::inc_type::WEAPON_SPEED},
      {u"incPAD", equip_game_instance::inc_type::WEAPON_PAD},
      {u"incMAD", equip_game_instance::inc_type::WEAPON_MAD},
      {u"incPDD", equip_game_instance::inc_type::PDD},
      {u"incACC", equip_game_instance::inc_type::ACC},
      {u"incSTR", equip_game_instance::inc_type::STR},
      {u"incDEX", equip_game_instance::inc_type::DEX},
      {u"incINT", equip_game_instance::inc_type::INT},
      {u"incLUK", equip_game_instance::inc_type::LUK},
  };
  for (auto [k, v] : incs) {
    if (equip_info->get_children()->contains(k)) {
      auto node = equip_info->get_child(k);
      r[v] = static_cast<wz::Property<int> *>(node)->get();
    }
  }

  return r;
}

uint8_t equip_game_instance::load_equip_tuc(const std::u16string &id) {
  auto equip_info = load_equip_info(id);
  return static_cast<wz::Property<int> *>(equip_info->get_child(u"tuc"))->get();
}

void equip_game_instance::add_equip(game_equip &equip,
                                    game_character &character, int slot) {
  auto type = load_equip_type(equip.id);
  if (type == u"Accessory") {
    character_game_instance::add_accessory(character, equip.id);
    character.accessory = equip;
  } else if (type == u"Cap") {
    character_game_instance::add_cap(character, equip.id);
    character.cap = equip;
  } else if (type == u"Cape") {
    character_game_instance::add_cape(character, equip.id);
    character.cape = equip;
  } else if (type == u"Coat") {
    character_game_instance::add_coat(character, equip.id);
    character.coat = equip;
  } else if (type == u"Glove") {
    character_game_instance::add_glove(character, equip.id);
    character.glove = equip;
  } else if (type == u"Longcoat") {
    character_game_instance::add_longcoat(character, equip.id);
    character.longcoat = equip;
  } else if (type == u"Pants") {
    character_game_instance::add_pants(character, equip.id);
    character.pant = equip;
  } else if (type == u"Ring") {
    if (slot == -1) {

    } else {
    }
  } else if (type == u"Shield") {
    character_game_instance::add_shield(character, equip.id);
    character.shield = equip;
  } else if (type == u"Shoes") {
    character_game_instance::add_shoes(character, equip.id);
    character.shoes = equip;
  } else if (type == u"Weapon") {
    character_game_instance::add_weapon(character, equip.id);
    character.weapon = equip;
  }
}

std::vector<game_equip>
equip_game_instance::load_equip_slot(game_equip &equip,
                                     game_character &character) {
  std::vector<game_equip> r;
  auto type = load_equip_type(equip.id);
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