#pragma once

#include "AnimatedSprite.h"
#include "Sound.h"
#include "FootHold.h"
#include "Attack.h"
#include "entt/entt.hpp"

#include <unordered_map>
#include <string>
#include <SDL3/SDL.h>

struct Mob
{
    std::unordered_map<std::u16string, AnimatedSprite> a;

    std::u16string index;

    // 状态机
    struct State
    {
        static const int STAND = 0;
        static const int MOVE = 1;
        static const int HIT = 2;
        static const int DIE = 3;
        static const int REMOVE = 4;
        static const int JUMP = 5;
        static const int FLY = 6;
        static const int ATTACK = 7;
    };
    // STAND,MOVE状态下循环的持续时间
    int tick = 500;

    int revive;
    // 复活时间
    int revive_time = 6000;
    int revive_alpha_time = 900;

    float init_x;
    float init_y;
    FootHold *init_fh;

    int hp = 100;
    int mp;

    int def = 10;
    int damage = 10;

    int state = State::STAND;

    std::u16string id;

    std::unordered_map<std::u16string, SoundWarp *> sounds;

    //仇恨目标
    entt::entity hit = entt::null;

    AnimatedSpriteWarp *atk = nullptr;
    AttackWarp atkw;

    SDL_FRect rect();
    SDL_FPoint head();
};
