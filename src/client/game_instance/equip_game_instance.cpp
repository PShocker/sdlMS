#include "equip_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <flat_map>
#include <string>

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