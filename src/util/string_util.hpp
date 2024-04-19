#ifndef HEADER_UTIL_STRING
#define HEADER_UTIL_STRING

#include <vector>
#include <ranges>
#include "wz/Property.hpp"

#include "template/currenton.hpp"

// 加载声音
namespace util
{
    class StringUtil final : public Currenton<StringUtil>
    {
    public:
        StringUtil();
        std::map<std::u16string, std::u16string> load_map_info(int mapId);
        std::map<std::u16string, std::u16string> load_npc_info(std::u16string npcId);

        static inline std::map<int, std::map<std::u16string, std::u16string>> _stringMap;
        static inline std::map<std::u16string, std::map<std::u16string, std::u16string>> _npcMap;
    };
}

#endif