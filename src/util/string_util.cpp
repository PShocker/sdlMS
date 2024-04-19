#include <string>
#include <locale>

#include "util/string_util.hpp"
#include "util/wz_util.hpp"

namespace util
{
    StringUtil::StringUtil()
    {
        wz::Node *root = WzUtil::current()->String->get_root();
        { // 解析地图String
            auto node = root->find_from_path(u"Map.img");
            for (auto &area : node->get_children())
            {
                for (auto &maps : area.second)
                {
                    for (auto &ids : maps->get_children())
                    {
                        std::map<std::u16string, std::u16string> m;

                        auto id_str = ids.first;
                        auto id = std::stoi(std::string{id_str.begin(), id_str.end()});

                        std::u16string mapName = u"";
                        std::u16string streetName = u"";
                        if (dynamic_cast<wz::Property<wz::wzstring> *>(ids.second[0]->find_from_path(u"mapName")) != nullptr)
                        {
                            mapName = dynamic_cast<wz::Property<wz::wzstring> *>(ids.second[0]->find_from_path(u"mapName"))->get();
                        }
                        if (dynamic_cast<wz::Property<wz::wzstring> *>(ids.second[0]->find_from_path(u"streetName")) != nullptr)
                        {
                            streetName = dynamic_cast<wz::Property<wz::wzstring> *>(ids.second[0]->find_from_path(u"streetName"))->get();
                        }
                        m.emplace(u"mapName", mapName);
                        m.emplace(u"streetName", streetName);

                        _stringMap.emplace(id, m);
                    }
                }
            }
        }
        {
            // 解析npc
            auto node = root->find_from_path(u"Npc.img");
            for (auto &_npc : node->get_children())
            {
                auto id_str = _npc.first;
                std::map<std::u16string, std::u16string> m;
                for (auto &npc : _npc.second[0]->get_children())
                {
                    auto val = dynamic_cast<wz::Property<wz::wzstring> *>(npc.second[0])->get();
                    m.emplace(npc.first,val);
                }
                _npcMap.emplace(id_str, m);
            }
        }
    }

    std::map<std::u16string, std::u16string> StringUtil::load_map_info(int mapId)
    {
        return _stringMap[mapId];
    }
    std::map<std::u16string, std::u16string> StringUtil::load_npc_info(std::u16string npcId)
    {
        return _npcMap[npcId];
    }
}