#include "util/hud_util.hpp"
#include "util/wz_util.hpp"
#include "sdlms/graphics.hpp"

#include "wz/Property.hpp"

namespace util
{

    std::map<std::string, Sprite> HudUtil::load_minimap()
    {
        std::map<std::string, Sprite> s;

        auto root = WzUtil::current()->UI->get_root();

        auto node = root->find_from_path(u"UIWindow.img/MiniMap/MaxMap");
        auto func = [](wz::Property<wz::WzCanvas> *canvas) -> Sprite
        {
            auto height = canvas->get().height;
            auto width = canvas->get().width;
            auto raw_data = canvas->get_raw_data();
            auto format = canvas->get().format;

            auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));
            auto ox = o->get().x;
            auto oy = o->get().y;

            SDL_FRect rect{(float)ox, (float)oy, (float)width, (float)height};
            Sprite sprite(raw_data, rect, (int)format);
            return sprite;
        };

        s.insert(std::make_pair("nw", func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"nw")))));
        s.insert(std::make_pair("ne", func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"ne")))));
        s.insert(std::make_pair("sw", func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"sw")))));
        s.insert(std::make_pair("se", func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"se")))));
        s.insert(std::make_pair("n", func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"n")))));
        s.insert(std::make_pair("s", func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"s")))));
        s.insert(std::make_pair("c", func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"c")))));
        s.insert(std::make_pair("w", func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w")))));
        s.insert(std::make_pair("e", func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e")))));

        return s;
    }
}