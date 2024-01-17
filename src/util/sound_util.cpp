#include "util/sound_util.hpp"
#include "util/wz_util.hpp"

namespace util
{
    std::vector<u8> SoundUtil::load_sound(int mapId)
    {
        wz::Node *root = WzUtil::current()->Map->get_root();
        std::string path = "Map/Map" + std::to_string(mapId / 100000000) + "/" + StringUtil::extend_id(mapId, 9) + ".img";
        auto node = root->find_from_path(path + "/info/bgm");
        if (node != nullptr)
        {
            auto url = dynamic_cast<wz::Property<wz::wzstring> *>(node)->get();
            url.insert(url.find('/'), u".img");
            node = WzUtil::current()->Sound->get_root()->find_from_path(url);
            auto sound = dynamic_cast<wz::Property<wz::WzSound> *>(node);
            auto data = sound->get_raw_data();
            return data;
        }
        return std::vector<u8>();
    }
}
