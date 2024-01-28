#include <string>
#include <locale>

#include "util/string_util.hpp"
#include "util/wz_util.hpp"

namespace util
{
    StringUtil::StringUtil()
    {

        wz::Node *root = WzUtil::current()->String->get_root();
        auto node = root->find_from_path(u"Map.img");
        for (auto &area : node->get_children())
        {
            for (auto &maps : area.second)
            {
                for (auto &ids : maps->get_children())
                {
                    auto id_str = ids.first;
                    auto id = std::stoi(std::string{id_str.begin(), id_str.end()});
                    auto mapName = dynamic_cast<wz::Property<wz::wzstring> *>(ids.second[0]->find_from_path(u"mapName"))->get();
                    auto streetName = dynamic_cast<wz::Property<wz::wzstring> *>(ids.second[0]->find_from_path(u"streetName"))->get();

                    std::u16string u16str = u"你好";
                    std::u8string u8str(reinterpret_cast<const char8_t *>(u16str.c_str()), u16str.size());
                    // _stringMap.insert(id,);
                    // std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
                    // std::wstring wstr = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.from_bytes(std::string_view(reinterpret_cast<const char *>(u8str.c_str()), u8str.size()));
                    printf("");
                }
            }
        }
    }
}