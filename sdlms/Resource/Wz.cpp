#include "Wz.h"

#ifdef __ANDROID__
#define USE_BUF
#endif

// #define USE_BUF

#ifdef USE_BUF
#include "Core/File.h"
#endif

#define U8 static_cast<u8>
#define IV4(A, B, C, D)            \
    {                              \
        U8(A), U8(B), U8(C), U8(D) \
    }

Wz::Wz(const std::string &filename_prefix)
{
    const auto iv = IV4(0xb9, 0x7d, 0x63, 0xe9);
    // 默认全部加载
#ifdef USE_BUF
    Character = (new wz::File(iv, File::buffer((std::string(filename_prefix) + std::string("Character.wz")).c_str()), File::size((std::string(filename_prefix) + std::string("Character.wz")).c_str())));
    Map = (new wz::File(iv, File::buffer((std::string(filename_prefix) + std::string("Map.wz")).c_str()), File::size((std::string(filename_prefix) + std::string("Map.wz")).c_str())));
    Mob = (new wz::File(iv, File::buffer((std::string(filename_prefix) + std::string("Mob.wz")).c_str()), File::size((std::string(filename_prefix) + std::string("Mob.wz")).c_str())));
    Npc = (new wz::File(iv, File::buffer((std::string(filename_prefix) + std::string("Npc.wz")).c_str()), File::size((std::string(filename_prefix) + std::string("Npc.wz")).c_str())));
    Sound = (new wz::File(iv, File::buffer((std::string(filename_prefix) + std::string("Sound.wz")).c_str()), File::size((std::string(filename_prefix) + std::string("Sound.wz")).c_str())));
    String = (new wz::File(iv, File::buffer((std::string(filename_prefix) + std::string("String.wz")).c_str()), File::size((std::string(filename_prefix) + std::string("String.wz")).c_str())));
#else
    // Base = (new wz::File(iv, (std::string(filename_prefix) + std::string("Base.wz")).c_str()));
    Character = (new wz::File(iv, (std::string(filename_prefix) + std::string("Character.wz")).c_str()));
    // Effect = (new wz::File(iv, (std::string(filename_prefix) + std::string("Effect.wz")).c_str()));
    // Etc = (new wz::File(iv, (std::string(filename_prefix) + std::string("Etc.wz")).c_str()));
    // Item = (new wz::File(iv, (std::string(filename_prefix) + std::string("Item.wz")).c_str()));
    // List = (new wz::File(iv, (std::string(filename_prefix) + std::string("List.wz")).c_str()));
    Map = (new wz::File(iv, (std::string(filename_prefix) + std::string("Map.wz")).c_str()));
    Mob = (new wz::File(iv, (std::string(filename_prefix) + std::string("Mob.wz")).c_str()));
    // Morph = (new wz::File(iv, (std::string(filename_prefix) + std::string("Morph.wz")).c_str()));
    Npc = (new wz::File(iv, (std::string(filename_prefix) + std::string("Npc.wz")).c_str()));
    // Quest = (new wz::File(iv, (std::string(filename_prefix) + std::string("Quest.wz")).c_str()));
    // Reactor = (new wz::File(iv, (std::string(filename_prefix) + std::string("Reactor.wz")).c_str()));
    // Skill = (new wz::File(iv, (std::string(filename_prefix) + std::string("Skill.wz")).c_str()));
    Sound = (new wz::File(iv, (std::string(filename_prefix) + std::string("Sound.wz")).c_str()));
    String = (new wz::File(iv, (std::string(filename_prefix) + std::string("String.wz")).c_str()));
    // TamingMob = (new wz::File(iv, (std::string(filename_prefix) + std::string("TamingMob.wz")).c_str()));
    // UI = (new wz::File(iv, (std::string(filename_prefix) + std::string("UI.wz")).c_str()));
#endif
    // Base->parse();
    Character->parse(u"Character");
    // Effect->parse();
    // Etc->parse();
    // Item->parse();
    // List->parse();
    Map->parse(u"Map");
    Mob->parse(u"Mob");
    // Morph->parse();
    Npc->parse(u"Npc");
    // Quest->parse();
    // Reactor->parse();
    // Skill->parse();
    Sound->parse(u"Sound");
    String->parse(u"String");
    // TamingMob->parse();
    // UI->parse(u"UI");
}