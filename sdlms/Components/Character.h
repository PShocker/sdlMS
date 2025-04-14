#pragma once
#include "Transform.h"
#include "Sprite.h"
#include "Core/Window.h"
#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include <unordered_set>
#include <tuple>

struct Character
{
    enum Action : uint8_t
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
        {u"alert", Action::ALERT},
        {u"dead", Action::DEAD},
        {u"fly", Action::FLY},
        {u"heal", Action::HEAL},
        {u"jump", Action::JUMP},
        {u"ladder", Action::LADDER},
        {u"prone", Action::PRONE},
        {u"proneStab", Action::PRONESTAB},
        {u"rope", Action::ROPE},
        {u"shot", Action::SHOT},
        {u"shoot1", Action::SHOOT1},
        {u"shoot2", Action::SHOOT2},
        {u"shootF", Action::SHOOTF},
        {u"sit", Action::SIT},
        {u"stabO1", Action::STABO1},
        {u"stabO2", Action::STABO2},
        {u"stabOF", Action::STABOF},
        {u"stabT1", Action::STABT1},
        {u"stabT2", Action::STABT2},
        {u"stabTF", Action::STABTF},
        {u"stand1", Action::STAND1},
        {u"stand2", Action::STAND2},
        {u"swingO1", Action::SWINGO1},
        {u"swingO2", Action::SWINGO2},
        {u"swingO3", Action::SWINGO3},
        {u"swingOF", Action::SWINGOF},
        {u"swingP1", Action::SWINGP1},
        {u"swingP2", Action::SWINGP2},
        {u"swingPF", Action::SWINGPF},
        {u"swingT1", Action::SWINGT1},
        {u"swingT2", Action::SWINGT2},
        {u"swingT3", Action::SWINGT3},
        {u"swingTF", Action::SWINGTF},
        {u"walk1", Action::WALK1},
        {u"walk2", Action::WALK2}};

    static inline const std::unordered_map<uint8_t, std::u16string> type_map2 = {
        {Action::ALERT, u"alert"},
        {Action::DEAD, u"dead"},
        {Action::FLY, u"fly"},
        {Action::HEAL, u"heal"},
        {Action::JUMP, u"jump"},
        {Action::LADDER, u"ladder"},
        {Action::PRONE, u"prone"},
        {Action::PRONESTAB, u"proneStab"},
        {Action::ROPE, u"rope"},
        {Action::SHOT, u"shot"},
        {Action::SHOOT1, u"shoot1"},
        {Action::SHOOT2, u"shoot2"},
        {Action::SHOOTF, u"shootF"},
        {Action::SIT, u"sit"},
        {Action::STABO1, u"stabO1"},
        {Action::STABO2, u"stabO2"},
        {Action::STABOF, u"stabOF"},
        {Action::STABT1, u"stabT1"},
        {Action::STABT2, u"stabT2"},
        {Action::STABTF, u"stabTF"},
        {Action::STAND1, u"stand1"},
        {Action::STAND2, u"stand2"},
        {Action::SWINGO1, u"swingO1"},
        {Action::SWINGO2, u"swingO2"},
        {Action::SWINGO3, u"swingO3"},
        {Action::SWINGOF, u"swingOF"},
        {Action::SWINGP1, u"swingP1"},
        {Action::SWINGP2, u"swingP2"},
        {Action::SWINGPF, u"swingPF"},
        {Action::SWINGT1, u"swingT1"},
        {Action::SWINGT2, u"swingT2"},
        {Action::SWINGT3, u"swingT3"},
        {Action::SWINGTF, u"swingTF"},
        {Action::WALK1, u"walk1"},
        {Action::WALK2, u"walk2"}};

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

    static inline std::unordered_map<uint8_t, SDL_FPoint> body_positions[Action::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> arm_positions[Action::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> hand_positions[Action::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> head_positions[Action::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> hair_positions[Action::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> face_positions[Action::LENGTH];
    static inline std::unordered_map<uint8_t, int32_t> stance_delays[Action::LENGTH];

    struct Wrap
    {
        uint8_t type;
        uint8_t frame;
        int delay;
    };

    static inline std::unordered_map<std::u16string, std::unordered_map<uint8_t, Wrap>>
        body_actions;
    static inline std::unordered_map<uint8_t, bool> show_face[Action::LENGTH];
    static inline wz::Node *character_node;

    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mobEquipFront[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> tamingMobFront[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mobEquipMid[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> saddleFront[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mobEquipUnderSaddle[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> tamingMobMid[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> characterStart[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> emotionOverBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponWristOverGlove[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capeOverHead[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponOverGlove[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWristOverHair[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveOverHair[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> handOverHair[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponOverHand[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> shieldOverHair[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWristBelowWeapon[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveBelowWeapon[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> handBelowWeapon[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponOverArm[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWristBelowMailArm[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailArmOverHair[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveBelowMailArm[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> armOverHair[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailArmOverHairBelowWeapon[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> armOverHairBelowWeapon[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponBelowArm[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capOverHair[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> hairOverHead[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryEyeOverCap[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capAccessory[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> cap[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> hair[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryEye[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryEyeShadow[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryFace[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capAccessoryBelowAccFace[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryEar[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capBelowAccessory[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryFaceOverFaceBelowCap[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> face[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> accessoryFaceBelowFace[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> hairShade[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> head[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> cape[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWrist[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailArm[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> glove[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> hand[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> arm[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weapon[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> shield[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponOverArmBelowHead[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWristBelowHead[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailArmBelowHeadOverMailChest[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveBelowHead[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> armBelowHeadOverMailChest[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailArmBelowHead[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> armBelowHead[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponOverBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailChestTop[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWristOverBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailChestOverHighest[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> pantsOverMailChest[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailChest[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> shoesTop[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> pantsOverShoesBelowMailChest[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> shoesOverPants[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailChestOverPants[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> pants[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> shoes[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> pantsBelowShoes[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mailChestBelowPants[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveOverBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> body[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveWristBelowBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> gloveBelowBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capAccessoryBelowBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> shieldBelowBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> capeBelowBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> hairBelowBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> weaponBelowBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHairOverCape[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backWing[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backWeaponOverShield[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backShield[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backCapOverHair[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHair[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backCap[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backWeaponOverHead[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHairBelowCapWide[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHairBelowCapNarrow[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHairBelowCap[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backCape[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backAccessoryFaceOverHead[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHead[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMailChestOverPants[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backPantsOverMailChest[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMailChest[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backPantsOverShoesBelowMailChest[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backShoes[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backPants[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backShoesBelowPants[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backPantsBelowShoes[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMailChestBelowPants[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backWeaponOverGlove[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backGloveWrist[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backGlove[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backAccessoryEar[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backAccessoryFace[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backCapAccessory[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMailChestAccessory[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backShieldBelowBody[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backHairBelowHead[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backWeapon[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> characterEnd[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> saddleRear[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> tamingMobRear[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> mobEquipRear[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMobEquipFront[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backTamingMobFront[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMobEquipMid[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backSaddle[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backMobEquipUnderSaddle[Action::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> backTamingMobMid[Action::LENGTH];
    std::unordered_map<std::u16string, std::unordered_map<uint8_t, std::pair<Transform, SpriteWarp *>> (*)[Action::LENGTH]> zmap;

    std::unordered_set<std::u16string> cap_vslot;

    uint8_t action = Action::JUMP;
    int action_index = 0;
    int action_time = 0;
    bool animate = true;
    std::u16string action_str = u"jump";
    uint8_t action_frame = 0;

    // use for attck and checkout wheather attack has finished
    bool animated = false;

    unsigned int invincible_cooldown = Window::dt_now;

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

    SDL_FRect r{-20, -50, 30, 45};
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
