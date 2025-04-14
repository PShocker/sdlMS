#pragma once

#include "Character.h"

#include <string>
#include <vector>
#include <array>

struct WeaponWrap
{
    std::u16string afterImage;
    std::u16string sfx;
    bool stand1 = false;
    bool walk1 = false;
    int attack = 0;
    int reqLevel = 0;
    std::u16string afterImage_index;
    WeaponWrap(const std::u16string &id);

    static bool if_long_range_weapon(int attack);

    enum Attack
    {
        NONE = 0,
        S1A1M1D = 1,
        SPEAR = 2,
        BOW = 3,
        CROSSBOW = 4,
        S2A2M2 = 5,
        WAND = 6,
        CLAW = 7,
        GUN = 9,
        NUM_ATTACKS
    };

    static inline const std::array<std::vector<Character::Action>, Attack::NUM_ATTACKS> degen_stances = {
        {{Character::Action::NONE},
         {Character::Action::NONE},
         {Character::Action::NONE},
         {Character::Action::SWINGT1, Character::Action::SWINGT3},
         {Character::Action::SWINGT1, Character::Action::STABT1},
         {Character::Action::NONE},
         {Character::Action::NONE},
         {Character::Action::STABO1, Character::Action::STABO2},
         {Character::Action::NONE},
         {Character::Action::SWINGP1, Character::Action::STABT2}}};

    static inline const std::array<std::vector<Character::Action>, Attack::NUM_ATTACKS> attack_stances = {
        {{Character::Action::NONE},
         {Character::Action::STABO1, Character::Action::STABO2, Character::Action::SWINGO1, Character::Action::SWINGO2, Character::Action::SWINGO3},
         {Character::Action::STABT1, Character::Action::SWINGP1},
         {Character::Action::SHOOT1},
         {Character::Action::SHOOT2},
         {Character::Action::STABO1, Character::Action::STABO2, Character::Action::SWINGT1, Character::Action::SWINGT2, Character::Action::SWINGT3},
         {Character::Action::SWINGO1, Character::Action::SWINGO2},
         {Character::Action::SWINGO1, Character::Action::SWINGO2},
         {Character::Action::NONE},
         {Character::Action::SHOT}}};
};
