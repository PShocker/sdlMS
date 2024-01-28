#ifndef HEADER_UTIL_STRING
#define HEADER_UTIL_STRING

#include <vector>
#include <ranges>
#include "wz/Property.hpp"

#include "util/currenton.hpp"

// 加载声音
namespace util
{
    class StringUtil final : public Currenton<StringUtil>
    {
    public:
        StringUtil();
        std::tuple<std::u16string, std::u16string> load_map_name(int mapId);

        std::map<int, std::tuple<std::u16string, std::u16string>> _stringMap;
    };
}

#endif