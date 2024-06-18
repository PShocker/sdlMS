#pragma once

#include <utility> // 包含 std::pair 头文件
#include "Transform.h"
#include "Sprite.h"
#include "wz/Property.hpp"

class Avatar : public Component
{
public:
    Avatar(){};
    ~Avatar();

public:
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

public:
    static inline std::unordered_map<uint8_t, SDL_FPoint> body_positions[ACTION::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> arm_positions[ACTION::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> hand_positions[ACTION::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> head_positions[ACTION::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> hair_positions[ACTION::LENGTH];
    static inline std::unordered_map<uint8_t, SDL_FPoint> face_positions[ACTION::LENGTH];
    static inline std::unordered_map<uint8_t, int32_t> stance_delays[ACTION::LENGTH];

    // std::unordered_map<std::string, std::unordered_map<uint8_t, BodyAction>> body_actions;
    static inline std::unordered_map<std::string, std::vector<int32_t>> attack_delays;
    static inline wz::Node *character_node;

public:
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> body[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> coat[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> coat_arm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> pants[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> head[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> face[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::vector<std::pair<Transform *, Sprite *>>> hairs[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> shoes[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> weapon[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> arm[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> hand[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> lHand[ACTION::LENGTH];
    std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> rHand[ACTION::LENGTH];
    uint8_t act = ACTION::JUMP;
    int act_index = 0;
    int act_time = 0;
    bool animate = true;

public:
    void add_body(const std::u16string &val);
    void add_coat(const std::u16string &val);
    void add_pants(const std::u16string &val);
    void add_head(const std::u16string &val);
    void add_face(const std::u16string &val);
    void add_hairs(const std::u16string &val);
    void add_shoes(const std::u16string &val);
    void add_weapon(const std::u16string &val);
    void add_arm(const std::u16string &val);
    void add_coat_arm(const std::u16string &val);
    void add_hand(const std::u16string &val);
    void add_lHand(const std::u16string &val);
    void add_rHand(const std::u16string &val);
    static Avatar *load();
    static inline bool inited = false;
    static void init(wz::Node *character_node);
};