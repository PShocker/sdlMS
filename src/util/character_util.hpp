#ifndef HEADER_UTIL_CHARACTER
#define HEADER_UTIL_CHARACTER

#include <vector>
#include <ranges>
#include <map>

#include "wz/Property.hpp"

#include "util/currenton.hpp"
#include "util/sprite_util.hpp"
#include "util/point.hpp"

#include "sdlms/character.hpp"

class Character;

namespace util
{
    class CharacterUtil final : public Currenton<CharacterUtil>
    {
    public:
        CharacterUtil();
        std::vector<Sprite> load();

    private:
        SpriteUtil *_sprite_util;

    private:
        wz::Node *_character_node;

    private:
        void load_body(wz::Node *node);

    public:
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
        enum Type : uint8_t
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
        static inline const std::map<std::u16string, Type> type_map = {
            {u"alert", Type::ALERT},
            {u"dead", Type::DEAD},
            {u"fly", Type::FLY},
            {u"heal", Type::HEAL},
            {u"jump", Type::JUMP},
            {u"ladder", Type::LADDER},
            {u"prone", Type::PRONE},
            {u"proneStab", Type::PRONESTAB},
            {u"rope", Type::ROPE},
            {u"shot", Type::SHOT},
            {u"shoot1", Type::SHOOT1},
            {u"shoot2", Type::SHOOT2},
            {u"shootF", Type::SHOOTF},
            {u"sit", Type::SIT},
            {u"stabO1", Type::STABO1},
            {u"stabO2", Type::STABO2},
            {u"stabOF", Type::STABOF},
            {u"stabT1", Type::STABT1},
            {u"stabT2", Type::STABT2},
            {u"stabTF", Type::STABTF},
            {u"stand1", Type::STAND1},
            {u"stand2", Type::STAND2},
            {u"swingO1", Type::SWINGO1},
            {u"swingO2", Type::SWINGO2},
            {u"swingO3", Type::SWINGO3},
            {u"swingOF", Type::SWINGOF},
            {u"swingP1", Type::SWINGP1},
            {u"swingP2", Type::SWINGP2},
            {u"swingPF", Type::SWINGPF},
            {u"swingT1", Type::SWINGT1},
            {u"swingT2", Type::SWINGT2},
            {u"swingT3", Type::SWINGT3},
            {u"swingTF", Type::SWINGTF},
            {u"walk1", Type::WALK1},
            {u"walk2", Type::WALK2}};

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

    private:
        std::unordered_map<uint8_t, Point<int32_t>> body_positions[Type::LENGTH];
        std::unordered_map<uint8_t, Point<int32_t>> arm_positions[Type::LENGTH];
        std::unordered_map<uint8_t, Point<int32_t>> hand_positions[Type::LENGTH];
        std::unordered_map<uint8_t, Point<int32_t>> head_positions[Type::LENGTH];
        std::unordered_map<uint8_t, Point<int32_t>> hair_positions[Type::LENGTH];
        std::unordered_map<uint8_t, Point<int32_t>> face_positions[Type::LENGTH];
        std::unordered_map<uint8_t, int32_t> stance_delays[Type::LENGTH];

        // std::unordered_map<std::string, std::unordered_map<uint8_t, BodyAction>> body_actions;
        std::unordered_map<std::string, std::vector<int32_t>> attack_delays;
    };
}

#endif