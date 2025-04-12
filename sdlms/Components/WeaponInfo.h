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

    static inline const std::array<std::vector<Character::ACTION>, Attack::NUM_ATTACKS> degen_stances = {
        {{Character::ACTION::NONE},
         {Character::ACTION::NONE},
         {Character::ACTION::NONE},
         {Character::ACTION::SWINGT1, Character::ACTION::SWINGT3},
         {Character::ACTION::SWINGT1, Character::ACTION::STABT1},
         {Character::ACTION::NONE},
         {Character::ACTION::NONE},
         {Character::ACTION::STABO1, Character::ACTION::STABO2},
         {Character::ACTION::NONE},
         {Character::ACTION::SWINGP1, Character::ACTION::STABT2}}};

    static inline const std::array<std::vector<Character::ACTION>, Attack::NUM_ATTACKS> attack_stances = {
        {{Character::ACTION::NONE},
         {Character::ACTION::STABO1, Character::ACTION::STABO2, Character::ACTION::SWINGO1, Character::ACTION::SWINGO2, Character::ACTION::SWINGO3},
         {Character::ACTION::STABT1, Character::ACTION::SWINGP1},
         {Character::ACTION::SHOOT1},
         {Character::ACTION::SHOOT2},
         {Character::ACTION::STABO1, Character::ACTION::STABO2, Character::ACTION::SWINGT1, Character::ACTION::SWINGT2, Character::ACTION::SWINGT3},
         {Character::ACTION::SWINGO1, Character::ACTION::SWINGO2},
         {Character::ACTION::SWINGO1, Character::ACTION::SWINGO2},
         {Character::ACTION::NONE},
         {Character::ACTION::SHOT}}};
};
