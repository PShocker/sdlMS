#pragma once
#include "Transform.h"
#include "Sprite.h"

#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include <unordered_set>
#include <tuple>

struct Character
{
    enum ACTION : uint8_t
    {
        NONE,
        ALERT,
        DEAD,
        FLY,
        HEAL,
        JUMP,
        LADDER,
        PRONE,
        PRONESTAB,
        ROPE,
        SHOT,
        SHOOT1,
        SHOOT2,
        SHOOTF,
        SIT,
        STABO1,
        STABO2,
        STABOF,
        STABT1,
        STABT2,
        STABTF,
        STAND1,
        STAND2,
        SWINGO1,
        SWINGO2,
        SWINGO3,
        SWINGOF,
        SWINGP1,
        SWINGP2,
        SWINGPF,
        SWINGT1,
        SWINGT2,
        SWINGT3,
        SWINGTF,
        WALK1,
        WALK2,
        LENGTH,
    };

    enum Layer : uint8_t
    {
        BODY,
        ARM,
        ARM_BELOW_HEAD,
        ARM_BELOW_HEAD_OVER_MAIL,
        ARM_OVER_HAIR,
        ARM_OVER_HAIR_BELOW_WEAPON,
        HAND_BELOW_WEAPON,
        HAND_OVER_HAIR,
        HAND_OVER_WEAPON,
        EAR,
        HEAD,
        HIGH_LEF_EAR,
        HUMAN_EAR,
        LEF_EAR,
        NUM_LAYERS
    };

    static inline const std::unordered_map<std::u16string, uint8_t> type_map = {
        {u"alert", ACTION::ALERT},
        {u"dead", ACTION::DEAD},
        {u"fly", ACTION::FLY},
        {u"heal", ACTION::HEAL},
        {u"jump", ACTION::JUMP},
        {u"ladder", ACTION::LADDER},
        {u"prone", ACTION::PRONE},
        {u"proneStab", ACTION::PRONESTAB},
        {u"rope", ACTION::ROPE},
        {u"shot", ACTION::SHOT},
        {u"shoot1", ACTION::SHOOT1},
        {u"shoot2", ACTION::SHOOT2},
        {u"shootF", ACTION::SHOOTF},
        {u"sit", ACTION::SIT},
        {u"stabO1", ACTION::STABO1},
        {u"stabO2", ACTION::STABO2},
        {u"stabOF", ACTION::STABOF},
        {u"stabT1", ACTION::STABT1},
        {u"stabT2", ACTION::STABT2},
        {u"stabTF", ACTION::STABTF},
        {u"stand1", ACTION::STAND1},
        {u"stand2", ACTION::STAND2},
        {u"swingO1", ACTION::SWINGO1},
        {u"swingO2", ACTION::SWINGO2},
        {u"swingO3", ACTION::SWINGO3},
        {u"swingOF", ACTION::SWINGOF},
        {u"swingP1", ACTION::SWINGP1},
        {u"swingP2", ACTION::SWINGP2},
        {u"swingPF", ACTION::SWINGPF},
        {u"swingT1", ACTION::SWINGT1},
        {u"swingT2", ACTION::SWINGT2},
        {u"swingT3", ACTION::SWINGT3},
        {u"swingTF", ACTION::SWINGTF},
        {u"walk1", ACTION::WALK1},
        {u"walk2", ACTION::WALK2}};

    static inline const std::unordered_map<uint8_t, std::u16string> type_map2 = {
        {ACTION::ALERT, u"alert"},
        {ACTION::DEAD, u"dead"},
        {ACTION::FLY, u"fly"},
        {ACTION::HEAL, u"heal"},
        {ACTION::JUMP, u"jump"},
        {ACTION::LADDER, u"ladder"},
        {ACTION::PRONE, u"prone"},
        {ACTION::PRONESTAB, u"proneStab"},
        {ACTION::ROPE, u"rope"},
        {ACTION::SHOT, u"shot"},
        {ACTION::SHOOT1, u"shoot1"},
        {ACTION::SHOOT2, u"shoot2"},
        {ACTION::SHOOTF, u"shootF"},
        {ACTION::SIT, u"sit"},
        {ACTION::STABO1, u"stabO1"},
        {ACTION::STABO2, u"stabO2"},
        {ACTION::STABOF, u"stabOF"},
        {ACTION::STABT1, u"stabT1"},
        {ACTION::STABT2, u"stabT2"},
        {ACTION::STABTF, u"stabTF"},
        {ACTION::STAND1, u"stand1"},
        {ACTION::STAND2, u"stand2"},
        {ACTION::SWINGO1, u"swingO1"},
        {ACTION::SWINGO2, u"swingO2"},
        {ACTION::SWINGO3, u"swingO3"},
        {ACTION::SWINGOF, u"swingOF"},
        {ACTION::SWINGP1, u"swingP1"},
        {ACTION::SWINGP2, u"swingP2"},
        {ACTION::SWINGPF, u"swingPF"},
        {ACTION::SWINGT1, u"swingT1"},
        {ACTION::SWINGT2, u"swingT2"},
        {ACTION::SWINGT3, u"swingT3"},
        {ACTION::SWINGTF, u"swingTF"},
        {ACTION::WALK1, u"walk1"},
        {ACTION::WALK2, u"walk2"}};

    static inline const std::unordered_map<std::u16string, Layer> layer_map =
        {
            {u"body", Layer::BODY},
            {u"backBody", Layer::BODY},
            {u"arm", Layer::ARM},
            {u"armBelowHead", Layer::ARM_BELOW_HEAD},
            {u"armBelowHeadOverMailChest", Layer::ARM_BELOW_HEAD_OVER_MAIL},
            {u"armOverHair", Layer::ARM_OVER_HAIR},
            {u"armOverHairBelowWeapon", Layer::ARM_OVER_HAIR_BELOW_WEAPON},
            {u"handBelowWeapon", Layer::HAND_BELOW_WEAPON},
            {u"handOverHair", Layer::HAND_OVER_HAIR},
            {u"handOverWeapon", Layer::HAND_OVER_WEAPON},
            {u"ear", Layer::EAR},
            {u"head", Layer::HEAD},
            {u"highlefEar", Layer::HIGH_LEF_EAR},
            {u"humanEar", Layer::HUMAN_EAR},
            {u"lefEar", Layer::LEF_EAR}};

    static inline std::unordered_map<uint8_t, SDL_FPoint> body_positions[ACTION::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> arm_positions[ACTION::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> hand_positions[ACTION::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> head_positions[ACTION::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> hair_positions[ACTION::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> face_positions[ACTION::LENGTH];
    static inline std::unordered_map<uint8_t, int32_t> stance_delays[ACTION::LENGTH];

    struct Action
    {
        uint8_t type;
        uint8_t frame;
        int delay;
    };

    static inline std::unordered_map<std::u16string, std::unordered_map<uint8_t, Action>>
        body_actions;
    static inline std::unordered_map<uint8_t, bool> show_face[ACTION::LENGTH];
    static inline wz::Node *character_node;

    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mobEquipFront[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> tamingMobFront[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mobEquipMid[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> saddleFront[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mobEquipUnderSaddle[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> tamingMobMid[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> characterStart[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> emotionOverBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponWristOverGlove[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capeOverHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponOverGlove[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWristOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> handOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponOverHand[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> shieldOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWristBelowWeapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveBelowWeapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> handBelowWeapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponOverArm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWristBelowMailArm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailArmOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveBelowMailArm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> armOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailArmOverHairBelowWeapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> armOverHairBelowWeapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponBelowArm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> hairOverHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryEyeOverCap[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capAccessory[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> cap[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> hair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryEye[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryEyeShadow[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryFace[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capAccessoryBelowAccFace[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryEar[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capBelowAccessory[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryFaceOverFaceBelowCap[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> face[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryFaceBelowFace[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> hairShade[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> head[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> cape[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWrist[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailArm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> glove[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> hand[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> arm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> shield[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponOverArmBelowHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWristBelowHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailArmBelowHeadOverMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveBelowHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> armBelowHeadOverMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailArmBelowHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> armBelowHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponOverBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailChestTop[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWristOverBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailChestOverHighest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> pantsOverMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> shoesTop[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> pantsOverShoesBelowMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> shoesOverPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailChestOverPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> pants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> shoes[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> pantsBelowShoes[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailChestBelowPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveOverBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> body[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWristBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capAccessoryBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> shieldBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capeBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> hairBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHairOverCape[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backWing[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backWeaponOverShield[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backShield[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backCapOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backCap[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backWeaponOverHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHairBelowCapWide[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHairBelowCapNarrow[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHairBelowCap[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backCape[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backAccessoryFaceOverHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMailChestOverPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backPantsOverMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backPantsOverShoesBelowMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backShoes[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backShoesBelowPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backPantsBelowShoes[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMailChestBelowPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backWeaponOverGlove[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backGloveWrist[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backGlove[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backAccessoryEar[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backAccessoryFace[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backCapAccessory[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMailChestAccessory[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backShieldBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHairBelowHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backWeapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> characterEnd[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> saddleRear[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> tamingMobRear[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mobEquipRear[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMobEquipFront[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backTamingMobFront[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMobEquipMid[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backSaddle[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMobEquipUnderSaddle[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backTamingMobMid[ACTION::LENGTH];
    std::unordered_map<std::u16string, std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> (*)[ACTION::LENGTH]> zmap;

    std::unordered_set<std::u16string> cap_vslot;

    int hp = 3000;
    int mp;

    uint8_t action = ACTION::JUMP;
    int action_index = 0;
    int action_time = 0;
    bool animate = true;
    std::u16string action_str = u"jump";
    uint8_t action_frame = 0;

    // use for attck and checkout wheather attack has finished
    bool animated = false;

    int invincible_cooldown = 0;

    std::u16string face_str;
    std::u16string face_type = u"default";
    std::u16string face_index = u"";
    unsigned int face_max_index = 0;
    int face_time = 0;

    void add_body(const std::u16string &val);
    void add_coat(const std::u16string &val);
    void add_cap(const std::u16string &val);
    void add_pants(const std::u16string &val);
    void add_head(const std::u16string &val);
    void add_face(const std::u16string &val, const std::u16string type = u"default", const std::u16string index = u"");
    void add_hairs(const std::u16string &val);
    void add_shoes(const std::u16string &val);
    void add_weapon(const std::u16string &val);
    void add_shield(const std::u16string &val);
    void add_cape(const std::u16string &val);

    Character();
    static void init();

    SDL_FRect r{-20, -40, 30, 30};
    float alpha = 255;

    // 状态机
    struct State
    {
        static const int STAND = 0;
        static const int WALK = 1;
        static const int JUMP = 2;
        static const int CLIMB = 3;
        static const int ATTACK = 4;
        static const int ALERT = 5;
        static const int PRONE = 6;
        static const int SKILL = 7;
        static const int DIE = 8;
        static const int SIT = 9;
    };
    int state = State::JUMP;
};
