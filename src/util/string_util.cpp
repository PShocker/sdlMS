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

                    auto mapName = dynamic_cast<wz::Property<wz::wzstring> *>(ids.second[0]->find_from_path(u"mapName"));
                    auto streetName = dynamic_cast<wz::Property<wz::wzstring> *>(ids.second[0]->find_from_path(u"streetName"));

                    std::tuple<std::u16string, std::u16string> tuple(mapName == nullptr ? u"" : mapName->get(),
                                                                     streetName == nullptr ? u"" : streetName->get());
                    _stringMap.insert(std::make_pair(id, tuple));
                }
            }
        }
    }

    std::tuple<std::u16string, std::u16string> StringUtil::load_map_name(int mapId){
        return _stringMap[mapId];
    }
}