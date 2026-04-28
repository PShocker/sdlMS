#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
#include <vector>

class character_extern_action {
public:
  std::u16string action;
  uint8_t frame;
  int32_t delay;
  SDL_FPoint move;
};

struct character_avatar {
  SDL_Texture *texture;
  SDL_FPoint origin;
  SDL_FPoint pos;
  std::u16string z;
};

struct character_avatar_render {
  std::flat_map<std::u16string, std::vector<std::vector<character_avatar>>>
      data;
};

class character_bone_data {
public:
  std::flat_map<std::u16string, SDL_FPoint> bone_pos;
};

class character_game_instance {
public:
  static void init_character_bone();

  static void load_self_character();
  static void load_server_character(uint32_t map_id);

  static fbs::CharacterT load_fbs_character();

  static inline game_character self;

  static inline float hforce = 0.0;
  static inline float vforce = 0.0;

  static void add_body(game_character &g, const std::u16string &val);
  static void add_coat(game_character &g, const std::u16string &val);
  static void add_cap(game_character &g, const std::u16string &val);
  static void add_pants(game_character &g, const std::u16string &val);
  static void add_head(game_character &g, const std::u16string &val);
  static void add_face(game_character &g, const std::u16string &val,
                       const std::u16string &type, const std::u16string &i);
  static void add_hairs(game_character &g, const std::u16string &val);
  static void add_shoes(game_character &g, const std::u16string &val);
  static void add_weapon(game_character &g, const std::u16string &val);
  static void add_shield(game_character &g, const std::u16string &val);
  static void add_cape(game_character &g, const std::u16string &val);

  static inline std::flat_map<uint64_t, game_character> others;

  static inline std::flat_map<std::u16string, character_extern_action>
      extern_action;

  static inline std::flat_map<std::u16string, std::vector<character_bone_data>>
      bone_data;

  static inline std::flat_map<std::u16string, character_avatar_render>
      avatar_data;

  const static inline std::vector<std::u16string> zmap = {
      u"Bd",
      u"Hd",
      u"Hr",
      u"Fc",
      u"At",
      u"Af",
      u"Am",
      u"Ae",
      u"As",
      u"Ay",
      u"Cp",
      u"Ri",
      u"Gv",
      u"Wp",
      u"Si",
      u"So",
      u"Pn",
      u"Ws",
      u"Ma",
      u"Wg",
      u"Sr",
      u"Tm",
      u"Sd",
      u"backTamingMobMid",
      u"backMobEquipUnderSaddle",
      u"backSaddle",
      u"backMobEquipMid",
      u"backTamingMobFront",
      u"backMobEquipFront",
      u"mobEquipRear",
      u"tamingMobRear",
      u"saddleRear",
      u"characterEnd",
      u"backWeapon",
      u"backHairBelowHead",
      u"backShieldBelowBody",
      u"backMailChestAccessory",
      u"backCapAccessory",
      u"backAccessoryFace",
      u"backAccessoryEar",
      u"backBody",
      u"backGlove",
      u"backGloveWrist",
      u"backWeaponOverGlove",
      u"backMailChestBelowPants",
      u"backPantsBelowShoes",
      u"backShoesBelowPants",
      u"backPants",
      u"backShoes",
      u"backPantsOverShoesBelowMailChest",
      u"backMailChest",
      u"backPantsOverMailChest",
      u"backMailChestOverPants",
      u"backHead",
      u"backAccessoryFaceOverHead",
      u"backAccessoryOverHead",
      u"backCape",
      u"backHairBelowCap",
      u"backHairBelowCapNarrow",
      u"backHairBelowCapWide",
      u"backWeaponOverHead",
      u"backCap",
      u"backHair",
      u"backCapOverHair",
      u"backShield",
      u"backWeaponOverShield",
      u"backWing",
      u"backHairOverCape",
      u"weaponBelowBody",
      u"hairBelowBody",
      u"capeBelowBody",
      u"shieldBelowBody",
      u"capAccessoryBelowBody",
      u"gloveBelowBody",
      u"gloveWristBelowBody",
      u"body",
      u"gloveOverBody",
      u"mailChestBelowPants",
      u"pantsBelowShoes",
      u"shoes",
      u"pants",
      u"mailChestOverPants",
      u"shoesOverPants",
      u"pantsOverShoesBelowMailChest",
      u"shoesTop",
      u"mailChest",
      u"pantsOverMailChest",
      u"mailChestOverHighest",
      u"gloveWristOverBody",
      u"mailChestTop",
      u"weaponOverBody",
      u"armBelowHead",
      u"mailArmBelowHead",
      u"armBelowHeadOverMailChest",
      u"gloveBelowHead",
      u"mailArmBelowHeadOverMailChest",
      u"gloveWristBelowHead",
      u"weaponOverArmBelowHead",
      u"shield",
      u"weapon",
      u"arm",
      u"hand",
      u"glove",
      u"mailArm",
      u"gloveWrist",
      u"cape",
      u"head",
      u"hairShade",
      u"accessoryFaceBelowFace",
      u"accessoryEyeBelowFace",
      u"face",
      u"accessoryFaceOverFaceBelowCap",
      u"capBelowAccessory",
      u"accessoryEar",
      u"capAccessoryBelowAccFace",
      u"accessoryFace",
      u"accessoryEyeShadow",
      u"accessoryEye",
      u"hair",
      u"cap",
      u"capAccessory",
      u"accessoryEyeOverCap",
      u"hairOverHead",
      u"accessoryOverHair",
      u"accessoryEarOverHair",
      u"capOverHair",
      u"weaponBelowArm",
      u"armOverHairBelowWeapon",
      u"mailArmOverHairBelowWeapon",
      u"armOverHair",
      u"gloveBelowMailArm",
      u"mailArmOverHair",
      u"gloveWristBelowMailArm",
      u"weaponOverArm",
      u"handBelowWeapon",
      u"gloveBelowWeapon",
      u"gloveWristBelowWeapon",
      u"shieldOverHair",
      u"weaponOverHand",
      u"handOverHair",
      u"gloveOverHair",
      u"gloveWristOverHair",
      u"weaponOverGlove",
      u"capeOverHead",
      u"weaponWristOverGlove",
      u"emotionOverBody",
      u"characterStart",
      u"backSaddleFront",
      u"saddleMid",
      u"tamingMobMid",
      u"mobEquipUnderSaddle",
      u"saddleFront",
      u"mobEquipMid",
      u"tamingMobFront",
      u"mobEquipFront"};
};