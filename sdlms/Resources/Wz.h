#pragma once

#include <wz/File.hpp>

struct Wz
{
    static inline wz::File *Base;
    static inline wz::File *Character;
    static inline wz::File *Effect;
    static inline wz::File *Etc;
    static inline wz::File *Item;
    static inline wz::File *List;
    static inline wz::File *Map;
    static inline wz::File *Mob;
    static inline wz::File *Morph;
    static inline wz::File *Npc;
    static inline wz::File *Quest;
    static inline wz::File *Reactor;
    static inline wz::File *Skill;
    static inline wz::File *Sound;
    static inline wz::File *String;
    static inline wz::File *TamingMob;
    static inline wz::File *UI;

    static void init(const std::string &filename_prefix);
};
