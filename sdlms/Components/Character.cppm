module;
#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include <unordered_set>

export module components:character;

import :sprite;
import :transform;

export struct Character
{
    enum ACTION : uint8_t
    {
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

    // std::unordered_map<std::string, std::unordered_map<uint8_t, BodyAction>> body_actions;
    static inline std::unordered_map<uint8_t, bool> show_face[ACTION::LENGTH];
    static inline std::unordered_map<std::string, std::vector<int32_t>> attack_delays;
    static inline wz::Node *character_node;

    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mobEquipFront[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> tamingMobFront[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mobEquipMid[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> saddleFront[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mobEquipUnderSaddle[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> tamingMobMid[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> characterStart[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> emotionOverBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> weaponWristOverGlove[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> capeOverHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> weaponOverGlove[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveWristOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> handOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> weaponOverHand[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> shieldOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveWristBelowWeapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveBelowWeapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> handBelowWeapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> weaponOverArm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveWristBelowMailArm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mailArmOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveBelowMailArm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> armOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mailArmOverHairBelowWeapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> armOverHairBelowWeapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> weaponBelowArm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> capOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> hairOverHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> accessoryEyeOverCap[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> capAccessory[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> cap[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> hair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> accessoryEye[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> accessoryEyeShadow[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> accessoryFace[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> capAccessoryBelowAccFace[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> accessoryEar[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> capBelowAccessory[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> accessoryFaceOverFaceBelowCap[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> face[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> accessoryFaceBelowFace[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> hairShade[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> head[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> cape[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveWrist[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mailArm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> glove[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> hand[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> arm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> weapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> shield[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> weaponOverArmBelowHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveWristBelowHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mailArmBelowHeadOverMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveBelowHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> armBelowHeadOverMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mailArmBelowHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> armBelowHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> weaponOverBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mailChestTop[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveWristOverBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mailChestOverHighest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> pantsOverMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> shoesTop[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> pantsOverShoesBelowMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> shoesOverPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mailChestOverPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> pants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> shoes[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> pantsBelowShoes[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mailChestBelowPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveOverBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> body[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveWristBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> gloveBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> capAccessoryBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> shieldBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> capeBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> hairBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> weaponBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backHairOverCape[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backWing[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backWeaponOverShield[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backShield[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backCapOverHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backHair[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backCap[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backWeaponOverHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backHairBelowCapWide[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backHairBelowCapNarrow[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backHairBelowCap[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backCape[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backAccessoryFaceOverHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backMailChestOverPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backPantsOverMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backPantsOverShoesBelowMailChest[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backShoes[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backShoesBelowPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backPantsBelowShoes[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backMailChestBelowPants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backWeaponOverGlove[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backGloveWrist[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backGlove[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backAccessoryEar[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backAccessoryFace[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backCapAccessory[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backMailChestAccessory[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backShieldBelowBody[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backHairBelowHead[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backWeapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> characterEnd[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> saddleRear[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> tamingMobRear[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> mobEquipRear[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backMobEquipFront[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backTamingMobFront[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backMobEquipMid[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backSaddle[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backMobEquipUnderSaddle[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> backTamingMobMid[ACTION::LENGTH];
    std::unordered_map<std::u16string, std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> (*)[ACTION::LENGTH]> zmap;

    std::unordered_set<std::u16string> cap_vslot;


    uint8_t action = ACTION::JUMP;
    int action_index = 0;
    int action_time = 0;
    bool animate = true;

    //use for attck and checkout wheather attack has finished
    bool animated = false;

    void add_body(const std::u16string &val);
    void add_coat(const std::u16string &val);
    void add_cap(const std::u16string &val);
    void add_pants(const std::u16string &val);
    void add_head(const std::u16string &val);
    void add_face(const std::u16string &val);
    void add_hairs(const std::u16string &val);
    void add_shoes(const std::u16string &val);
    void add_weapon(const std::u16string &val);
    void add_shield(const std::u16string &val);

    Character();
    static void init();

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
    };
    int state = State::JUMP;
};
