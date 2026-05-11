#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
#include <vector>

class character_extern_action {
public:
  std::u16string action;
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
  std::u16string islot;
  std::flat_set<std::u16string> vslot;
  std::flat_map<std::u16string, std::vector<std::vector<character_avatar>>>
      data;
};

struct character_face_render {
  // defalut ,stand0,0,0,0
  std::flat_map<std::u16string, character_avatar_render> data;
};

struct character_other_data {
  game_character g_character;
  std::vector<fbs::MovementT> movements;
};

class character_bone_data {
public:
  std::flat_map<std::u16string, SDL_FPoint> bone_pos;
  std::flat_map<std::u16string, SDL_FPoint> part_pos;

  uint32_t delay;
  uint8_t face;
};

class character_game_instance {
private:
  static std::flat_set<std::u16string> split_vslot(const std::u16string &vslot);

public:
  static void init_character_bone();

  static void load_self_pos(const std::u16string &pn, uint8_t index);
  static void load_self_character();
  static void
  load_others_character(const std::vector<std::unique_ptr<fbs::PlayerT>> &v);
  static void load_others_character(const std::unique_ptr<fbs::PlayerT> &c);
  static fbs::CharacterT load_self_fbs_character();

  // network
  static void server_character_move(const fbs::ServerCharacterMoveT &r);

  static inline game_character self;

  static void add_body(game_character &g, const std::u16string &val);
  static void add_coat(game_character &g, const std::u16string &val);
  static void add_cap(game_character &g, const std::u16string &val);
  static void add_pants(game_character &g, const std::u16string &val);
  static void add_head(game_character &g, const std::u16string &val);
  static void add_face(game_character &g, const std::u16string &val);
  static void add_hair(game_character &g, const std::u16string &val);
  static void add_shoes(game_character &g, const std::u16string &val);
  static void add_weapon(game_character &g, const std::u16string &val);
  static void add_shield(game_character &g, const std::u16string &val);
  static void add_cape(game_character &g, const std::u16string &val);

  static inline std::flat_map<uint64_t, character_other_data> others;

  static inline std::flat_map<std::u16string,
                              std::vector<character_extern_action>>
      extern_action;

  // action index
  static inline std::flat_map<std::u16string, std::vector<character_bone_data>>
      bone_data;

  static inline std::flat_map<std::u16string, character_avatar_render>
      avatar_data;

  static inline std::flat_map<std::u16string, character_face_render> face_data;

  const static inline std::vector<std::u16string> zmap = {
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

  const static inline std::vector<std::u16string> zmap2 = {
      u"Sd", u"Tm", u"Sr", u"Wg", u"Ma", u"Ws", u"Pn", u"So",
      u"Si", u"Wp", u"Gv", u"Ri", u"Cp", u"Ay", u"As", u"Ae",
      u"Am", u"Af", u"At", u"Fc", u"Hr", u"Hd", u"Bd",
  };

  const static inline std::flat_map<std::u16string,
                                    std::flat_set<std::u16string>>
      smap = {
          {u"weaponWristOverGlove", {u"Wp"}},
          {u"capeOverHead", {u"Sr"}},
          {u"weaponOverGlove", {u"Wp"}},
          {u"gloveWristOverHair", {u"Gw"}},
          {u"gloveOverHair", {u"Gl"}},
          {u"handOverHair", {u"Bd"}},
          {u"weaponOverHand", {u"Wp"}},
          {u"shieldOverHair", {u"Si"}},
          {u"gloveWristBelowWeapon", {u"Gw"}},
          {u"gloveBelowWeapon", {u"Gl"}},
          {u"handBelowWeapon", {u"Bd"}},
          {u"weaponOverArm", {u"Wp"}},
          {u"gloveWristBelowMailArm", {u"Gw"}},
          {u"mailArmOverHair", {u"Ma", u"Gw"}},
          {u"gloveBelowMailArm", {u"Gl"}},
          {u"armOverHair", {u"Bd"}},
          {u"mailArmOverHairBelowWeapon", {u"Ma", u"Gw"}},
          {u"armOverHairBelowWeapon", {u"Bd"}},
          {u"weaponBelowArm", {u"Wp"}},
          {u"capOverHair",
           {u"Cp", u"Hd", u"H1", u"H2", u"H3", u"H4", u"H5", u"H6", u"Hs",
            u"Hf", u"Hb", u"Af", u"Ay", u"As", u"Ae"}},
          {u"accessoryEarOverHair", {u"Ae"}},
          {u"accessoryOverHair",
           {u"Cp", u"Hd", u"H1", u"H2", u"H3", u"H4", u"H5", u"H6", u"Hs",
            u"Hf", u"Hb", u"Af", u"Ay", u"As", u"Ae"}},
          {u"hairOverHead", {u"H1"}},
          {u"accessoryEyeOverCap", {u"Af"}},
          {u"capAccessory", {u"Cc"}},
          {u"cap",
           {u"Cp", u"Hd", u"H1", u"H2", u"H3", u"H4", u"H5", u"H6", u"Hs",
            u"Hf", u"Hb", u"Af", u"Ay", u"As", u"Ae"}},
          {u"hair", {u"H2"}},
          {u"accessoryEye", {u"Ay"}},
          {u"accessoryEyeShadow", {u"As"}},
          {u"accessoryFace", {u"Af"}},
          {u"capAccessoryBelowAccFace", {u"Cc"}},
          {u"accessoryEar", {u"Ae"}},
          {u"capBelowAccessory",
           {u"Cp", u"Hd", u"H1", u"H2", u"H3", u"H4", u"H5", u"H6", u"Hs",
            u"Hf", u"Hb", u"Af", u"Ay", u"As", u"Ae"}},
          {u"accessoryFaceOverFaceBelowCap", {u"Af"}},
          {u"face", {u"Fc"}},
          {u"accessoryFaceBelowFace", {u"Af"}},
          {u"hairShade", {u"Hs"}},
          {u"head", {u"Hd"}},
          {u"cape", {u"Sr"}},
          {u"gloveWrist", {u"Gw"}},
          {u"mailArm", {u"Ma", u"Gw"}},
          {u"glove", {u"Gl"}},
          {u"hand", {u"Bd"}},
          {u"arm", {u"Bd"}},
          {u"weapon", {u"Wp"}},
          {u"shield", {u"Si"}},
          {u"weaponOverArmBelowHead", {u"Wp"}},
          {u"gloveWristBelowHead", {u"Gw"}},
          {u"mailArmBelowHeadOverMailChest", {u"Ma", u"Gw"}},
          {u"gloveBelowHead", {u"Gl"}},
          {u"armBelowHeadOverMailChest", {u"Bd"}},
          {u"mailArmBelowHead", {u"Ma", u"Gw"}},
          {u"armBelowHead", {u"Bd"}},
          {u"weaponOverBody", {u"Wp"}},
          {u"mailChestTop", {u"Ma", u"Gw"}},
          {u"gloveWristOverBody", {u"Gw"}},
          {u"mailChestOverHighest", {u"Ma", u"Gw"}},
          {u"pantsOverMailChest", {u"Pn", u"So"}},
          {u"shoesTop", {u"So"}},
          {u"mailChest", {u"Ma", u"Gw"}},
          {u"shoesOverPants", {u"So"}},
          {u"mailChestOverPants", {u"Ma", u"Gw"}},
          {u"pants", {u"Pn", u"So"}},
          {u"shoes", {u"So"}},
          {u"pantsBelowShoes", {u"Pn", u"So"}},
          {u"mailChestBelowPants", {u"Ma", u"Gw"}},
          {u"gloveOverBody", {u"Gl"}},
          {u"body", {u"Bd"}},
          {u"gloveWristBelowBody", {u"Gw"}},
          {u"gloveBelowBody", {u"Gl"}},
          {u"capAccessoryBelowBody", {u"Cc"}},
          {u"shieldBelowBody", {u"Si"}},
          {u"capeBelowBody", {u"Sr"}},
          {u"hairBelowBody", {u"Hb"}},
          {u"weaponBelowBody", {u"Wp"}},
          {u"backHairOverCape", {u"Hc"}},
          {u"backWing", {u"Wg"}},
          {u"backWeaponOverShield", {u"Wp"}},
          {u"backShield", {u"Si"}},
          {u"backCapOverHair",
           {u"Cp", u"Hd", u"H1", u"H2", u"H3", u"H4", u"H5", u"H6", u"Hs",
            u"Hf", u"Hb", u"Af", u"Ay", u"As", u"Ae"}},
          {u"backHair", {u"H1"}},
          {u"backCap",
           {u"Cp", u"Hd", u"H1", u"H2", u"H3", u"H4", u"H5", u"H6", u"Hs",
            u"Hf", u"Hb", u"Af", u"Ay", u"As", u"Ae"}},
          {u"backWeaponOverHead", {u"Wp"}},
          {u"backHairBelowCapWide", {u"H4"}},
          {u"backHairBelowCapNarrow", {u"H5"}},
          {u"backHairBelowCap", {u"H6"}},
          {u"backCape", {u"Sr"}},
          {u"backAccessoryOverHead", {u"Bd"}},
          {u"backAccessoryFaceOverHead", {u"Af"}},
          {u"backHead", {u"Bd"}},
          {u"backMailChestOverPants", {u"Ma", u"Gw"}},
          {u"backPantsOverMailChest", {u"Pn", u"So"}},
          {u"backMailChest", {u"Ma", u"Gw"}},
          {u"backShoes", {u"So"}},
          {u"backPants", {u"Pn", u"So"}},
          {u"backShoesBelowPants", {u"So"}},
          {u"backPantsBelowShoes", {u"Pn", u"So"}},
          {u"backMailChestBelowPants", {u"Ma", u"Gw"}},
          {u"backWeaponOverGlove", {u"Wp"}},
          {u"backGloveWrist", {u"Gw"}},
          {u"backGlove", {u"Gl"}},
          {u"backBody", {u"Bd"}},
          {u"backAccessoryEar", {u"Ae"}},
          {u"backAccessoryFace", {u"Af"}},
          {u"backCapAccessory", {u"Cc", u"H5"}},
          {u"backMailChestAccessory", {u"Ma"}},
          {u"backShieldBelowBody", {u"Si"}},
          {u"backHairBelowHead", {u"Hf"}},
          {u"backWeapon", {u"Wp"}},
          {u"accessoryEyeBelowFace", {u"Ay"}},
  };
};