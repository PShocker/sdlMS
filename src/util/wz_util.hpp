#pragma once
#include <wz/File.hpp>
#include <memory>
#include "string_util.hpp"

namespace WzUtil
{
	const std::string filename_prefix = "C:/Users/Shocker/Desktop/sdlMS/data/";

	extern wz::File *Base;
	extern wz::File *Character;
	extern wz::File *Effect;
	extern wz::File *Etc;
	extern wz::File *Item;
	extern wz::File *List;
	extern wz::File *Map;
	extern wz::File *Mob;
	extern wz::File *Morph;
	extern wz::File *Npc;
	extern wz::File *Quest;
	extern wz::File *Reactor;
	extern wz::File *Skill;
	extern wz::File *Sound;
	extern wz::File *String;
	extern wz::File *TamingMob;
	extern wz::File *UI;

	void init();

	std::u16string get_map_node_path(int32_t mapid);
}; // namespace WzFiles