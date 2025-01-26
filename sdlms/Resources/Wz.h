#pragma once

#include <wz/File.hpp>
#include <wz/Img.hpp>

struct Wz
{
#ifdef __EMSCRIPTEN__
    static inline wz::Img *Base;
    static inline wz::Img *Character;
    static inline wz::Img *Effect;
    static inline wz::Img *Etc;
    static inline wz::Img *Item;
    static inline wz::Img *List;
    static inline wz::Img *Map;
    static inline wz::Img *Mob;
    static inline wz::Img *Morph;
    static inline wz::Img *Npc;
    static inline wz::Img *Quest;
    static inline wz::Img *Reactor;
    static inline wz::Img *Skill;
    static inline wz::Img *Sound;
    static inline wz::Img *String;
    static inline wz::Img *TamingMob;
    static inline wz::Img *UI;

#else
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
#endif
    static void init(const std::string &filename_prefix);
};
