#include "Wz.h"

void Wz::init(const std::string &filename_prefix)
{
    const auto iv = IV4(0xb9, 0x7d, 0x63, 0xe9);
    // Base = (new wz::File(iv, (std::string(filename_prefix) + std::string("Base.wz")).c_str()));
    Character = (new wz::File(iv, (std::string(filename_prefix) + std::string("Character.wz")).c_str()));
    Effect = (new wz::File(iv, (std::string(filename_prefix) + std::string("Effect.wz")).c_str()));
    Etc = (new wz::File(iv, (std::string(filename_prefix) + std::string("Etc.wz")).c_str()));
    Item = (new wz::File(iv, (std::string(filename_prefix) + std::string("Item.wz")).c_str()));
    // List = (new wz::File(iv, (std::string(filename_prefix) + std::string("List.wz")).c_str()));
    Map = (new wz::File(iv, (std::string(filename_prefix) + std::string("Map.wz")).c_str()));
    Mob = (new wz::File(iv, (std::string(filename_prefix) + std::string("Mob.wz")).c_str()));
    // Morph = (new wz::File(iv, (std::string(filename_prefix) + std::string("Morph.wz")).c_str()));
    Npc = (new wz::File(iv, (std::string(filename_prefix) + std::string("Npc.wz")).c_str()));
    // Quest = (new wz::File(iv, (std::string(filename_prefix) + std::string("Quest.wz")).c_str()));
    Reactor = (new wz::File(iv, (std::string(filename_prefix) + std::string("Reactor.wz")).c_str()));
    Skill = (new wz::File(iv, (std::string(filename_prefix) + std::string("Skill.wz")).c_str()));
    Sound = (new wz::File(iv, (std::string(filename_prefix) + std::string("Sound.wz")).c_str()));
    String = (new wz::File(iv, (std::string(filename_prefix) + std::string("String.wz")).c_str()));
    // TamingMob = (new wz::File(iv, (std::string(filename_prefix) + std::string("TamingMob.wz")).c_str()));
    UI = (new wz::File(iv, (std::string(filename_prefix) + std::string("UI.wz")).c_str()));

    // Base->parse(u"Base");
    Character->parse(u"Character");
    Effect->parse(u"Effect");
    Etc->parse(u"Etc");
    Item->parse(u"Item");
    // List->parse();
    Map->parse(u"Map");
    Mob->parse(u"Mob");
    // Morph->parse();
    Npc->parse(u"Npc");
    // Quest->parse();
    Reactor->parse(u"Reactor");
    Skill->parse(u"Skill");
    Sound->parse(u"Sound");
    String->parse(u"String");
    // TamingMob->parse();
    UI->parse(u"UI");
}