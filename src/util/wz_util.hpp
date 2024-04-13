#ifndef HEADER_UTIL_WZUTIL
#define HEADER_UTIL_WZUTIL

#include <wz/File.hpp>
#include <memory>
#include "string_util.hpp"
#include "template/currenton.hpp"

namespace util
{
	class WzUtil final : public Currenton<WzUtil>
	{
	public:
		WzUtil();

	public:
		const std::string filename_prefix = "./data/";
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

	}; // namespace WzFiles
}
#endif