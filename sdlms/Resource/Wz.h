#pragma once
#include <wz/File.hpp>
#include "Resource.h"
class Wz : public Resource
{
public:
    Wz(const std::string &filename_prefix);

public:
    wz::File *Base;
    wz::File *Character;
    wz::File *Effect;
    wz::File *Etc;
    wz::File *Item;
    wz::File *List;
    wz::File *Map;
    wz::File *Mob;
    wz::File *Morph;
    wz::File *Npc;
    wz::File *Quest;
    wz::File *Reactor;
    wz::File *Skill;
    wz::File *Sound;
    wz::File *String;
    wz::File *TamingMob;
    wz::File *UI;
};
