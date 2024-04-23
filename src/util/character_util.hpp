#ifndef HEADER_UTIL_CHARACTER
#define HEADER_UTIL_CHARACTER

#include <vector>
#include <ranges>
#include <map>

#include "wz/Property.hpp"

#include "template/currenton.hpp"
#include "template/point.hpp"

#include "sdlms/character.hpp"

namespace util
{
    class CharacterUtil final : public Currenton<CharacterUtil>
    {
    public:
        CharacterUtil();
        std::unordered_map<uint8_t, std::vector<std::tuple<std::vector<Sprite>,int>>> load();

    private:
        wz::Node *_character_node;

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

        static inline const std::unordered_map<std::u16string, Character::Type> type_map = {
            {u"alert", Character::Type::ALERT},
            {u"dead", Character::Type::DEAD},
            {u"fly", Character::Type::FLY},
            {u"heal", Character::Type::HEAL},
            {u"jump", Character::Type::JUMP},
            {u"ladder", Character::Type::LADDER},
            {u"prone", Character::Type::PRONE},
            {u"proneStab", Character::Type::PRONESTAB},
            {u"rope", Character::Type::ROPE},
            {u"shot", Character::Type::SHOT},
            {u"shoot1", Character::Type::SHOOT1},
            {u"shoot2", Character::Type::SHOOT2},
            {u"shootF", Character::Type::SHOOTF},
            {u"sit", Character::Type::SIT},
            {u"stabO1", Character::Type::STABO1},
            {u"stabO2", Character::Type::STABO2},
            {u"stabOF", Character::Type::STABOF},
            {u"stabT1", Character::Type::STABT1},
            {u"stabT2", Character::Type::STABT2},
            {u"stabTF", Character::Type::STABTF},
            {u"stand1", Character::Type::STAND1},
            {u"stand2", Character::Type::STAND2},
            {u"swingO1", Character::Type::SWINGO1},
            {u"swingO2", Character::Type::SWINGO2},
            {u"swingO3", Character::Type::SWINGO3},
            {u"swingOF", Character::Type::SWINGOF},
            {u"swingP1", Character::Type::SWINGP1},
            {u"swingP2", Character::Type::SWINGP2},
            {u"swingPF", Character::Type::SWINGPF},
            {u"swingT1", Character::Type::SWINGT1},
            {u"swingT2", Character::Type::SWINGT2},
            {u"swingT3", Character::Type::SWINGT3},
            {u"swingTF", Character::Type::SWINGTF},
            {u"walk1", Character::Type::WALK1},
            {u"walk2", Character::Type::WALK2}};

        static inline const std::unordered_map<uint8_t, std::u16string> _type_map = {
            {Character::Type::ALERT, u"alert"},
            {Character::Type::DEAD, u"dead"},
            {Character::Type::FLY, u"fly"},
            {Character::Type::HEAL, u"heal"},
            {Character::Type::JUMP, u"jump"},
            {Character::Type::LADDER, u"ladder"},
            {Character::Type::PRONE, u"prone"},
            {Character::Type::PRONESTAB, u"proneStab"},
            {Character::Type::ROPE, u"rope"},
            {Character::Type::SHOT, u"shot"},
            {Character::Type::SHOOT1, u"shoot1"},
            {Character::Type::SHOOT2, u"shoot2"},
            {Character::Type::SHOOTF, u"shootF"},
            {Character::Type::SIT, u"sit"},
            {Character::Type::STABO1, u"stabO1"},
            {Character::Type::STABO2, u"stabO2"},
            {Character::Type::STABOF, u"stabOF"},
            {Character::Type::STABT1, u"stabT1"},
            {Character::Type::STABT2, u"stabT2"},
            {Character::Type::STABTF, u"stabTF"},
            {Character::Type::STAND1, u"stand1"},
            {Character::Type::STAND2, u"stand2"},
            {Character::Type::SWINGO1, u"swingO1"},
            {Character::Type::SWINGO2, u"swingO2"},
            {Character::Type::SWINGO3, u"swingO3"},
            {Character::Type::SWINGOF, u"swingOF"},
            {Character::Type::SWINGP1, u"swingP1"},
            {Character::Type::SWINGP2, u"swingP2"},
            {Character::Type::SWINGPF, u"swingPF"},
            {Character::Type::SWINGT1, u"swingT1"},
            {Character::Type::SWINGT2, u"swingT2"},
            {Character::Type::SWINGT3, u"swingT3"},
            {Character::Type::SWINGTF, u"swingTF"},
            {Character::Type::WALK1, u"walk1"},
            {Character::Type::WALK2, u"walk2"}};

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
        std::unordered_map<uint8_t, Point<int32_t>> body_positions[Character::Type::LENGTH];
        std::unordered_map<uint8_t, Point<int32_t>> arm_positions[Character::Type::LENGTH];
        std::unordered_map<uint8_t, Point<int32_t>> hand_positions[Character::Type::LENGTH];
        std::unordered_map<uint8_t, Point<int32_t>> head_positions[Character::Type::LENGTH];
        std::unordered_map<uint8_t, Point<int32_t>> hair_positions[Character::Type::LENGTH];
        std::unordered_map<uint8_t, Point<int32_t>> face_positions[Character::Type::LENGTH];
        std::unordered_map<uint8_t, int32_t> stance_delays[Character::Type::LENGTH];

        // std::unordered_map<std::string, std::unordered_map<uint8_t, BodyAction>> body_actions;
        std::unordered_map<std::string, std::vector<int32_t>> attack_delays;
    };
}

#endif