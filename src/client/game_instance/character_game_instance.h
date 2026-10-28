#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
#include <vector>
namespace sdlms {


using namespace fbs;

struct CharacterExternAction{
  std::u16string action;
  int8_t frame;
  int32_t delay;
  SDL_FPoint move;
};

struct CharacterAvatar {
  SDL_Texture *texture = nullptr;
  SDL_FPoint origin;
  SDL_FPoint pos;
  std::u16string z;
  std::u16string name = u"";
};

struct CharacterAvatarRender {
  std::u16string islot;
  std::flat_set<std::u16string> vslot;
  std::flat_map<std::u16string, std::vector<std::vector<CharacterAvatar>>>
      data;
};

struct CharacterFaceRender {
  // defalut ,stand0,0,0,0
  std::flat_map<std::u16string, CharacterAvatarRender> data;
  std::flat_map<std::u16string, std::vector<uint32_t>> delay;
};

struct CharacterOtherData {
  GameCharacter g_character;
  std::vector<MovementT> mvs;
  PlayerT player_t;
};

struct CharacterBoneData{
  std::flat_map<std::u16string, SDL_FPoint> bone_pos;
  std::flat_map<std::u16string, SDL_FPoint> part_pos;

  uint32_t delay;
  uint8_t face;
};

class CharacterGameInstance {
private:
  static std::flat_set<std::u16string> SplitVslot(const std::u16string &vslot);
  static std::u16string SplitIslot(const std::u16string &islot);
  static void AddAvatarCache(const std::u16string &val,
                               const std::u16string &sub);

public:
  static inline GameCharacter self_;

  static void InitCharacterBone();
  static void InitDefaultClothes();

  static void LoadSelfCharacter();

  static void AddBody(GameCharacter &g, const std::u16string &val);
  static void AddCoat(GameCharacter &g, const std::u16string &val);
  static void AddCap(GameCharacter &g, const std::u16string &val);
  static void AddPants(GameCharacter &g, const std::u16string &val);
  static void AddHead(GameCharacter &g, const std::u16string &val);
  static void AddEar(GameCharacter &g, const std::u16string &val);
  static void AddFace(GameCharacter &g, const std::u16string &val);
  static void AddHair(GameCharacter &g, const std::u16string &val);
  static void AddShoes(GameCharacter &g, const std::u16string &val);
  static void AddWeapon(GameCharacter &g, const std::u16string &val);
  static void AddShield(GameCharacter &g, const std::u16string &val);
  static void AddCape(GameCharacter &g, const std::u16string &val);
  static void AddAccessory(GameCharacter &g, const std::u16string &val);
  static void AddGlove(GameCharacter &g, const std::u16string &val);
  static void AddLongcoat(GameCharacter &g, const std::u16string &val);

  static void AddCoatDeco(GameCharacter &g, const std::u16string &val);
  static void AddCapDeco(GameCharacter &g, const std::u16string &val);
  static void AddPantsDeco(GameCharacter &g, const std::u16string &val);
  static void AddShoesDeco(GameCharacter &g, const std::u16string &val);
  static void AddShieldDeco(GameCharacter &g, const std::u16string &val);
  static void AddCapeDeco(GameCharacter &g, const std::u16string &val);
  static void AddAccessoryDeco(GameCharacter &g, const std::u16string &val);
  static void AddGloveDeco(GameCharacter &g, const std::u16string &val);
  static void AddLongcoatDeco(GameCharacter &g, const std::u16string &val);
  static void AddWeaponDeco(GameCharacter &g, const std::u16string &val);

  static void AddRing0Deco(GameCharacter &g, const std::u16string &val);
  static void AddRing1Deco(GameCharacter &g, const std::u16string &val);

  static inline std::flat_map<uint64_t, CharacterOtherData> others_;

  static inline std::flat_map<std::u16string,
                              std::vector<CharacterExternAction>>
      extern_action_;

  // action index
  static inline std::flat_map<std::u16string, std::vector<CharacterBoneData>>
      bone_data_;

  static inline std::flat_map<std::u16string, CharacterAvatarRender>
      avatar_data_;

  static inline std::flat_map<std::u16string, CharacterFaceRender> face_data_;

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

}  // namespace sdlms
