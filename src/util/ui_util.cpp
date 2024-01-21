#include "util/ui_util.hpp"
#include "util/wz_util.hpp"
#include "sdlms/graphics.hpp"

#include "wz/Property.hpp"

namespace util
{

    std::vector<Sprite> UIUtil::load_minimap()
    {
        std::vector<Sprite> v_s;

        auto root = WzUtil::current()->UI->get_root();

        auto node = root->find_from_path(u"UIWindow.img/MiniMap");
        auto nw = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"MinMap/nw"));
        auto height = nw->get().height;
        auto width = nw->get().width;
        auto raw_data = nw->get_raw_data();
        auto format = nw->get().format;

        auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(nw->get_child(u"origin"));
        auto ox = o->get().x;
        auto oy = o->get().y;

        SDL_FRect rect{(float)ox, (float)oy, (float)width, (float)height};

        Sprite sprite(raw_data, rect, (int)format);

        v_s.push_back(sprite);

        auto ne = node->find_from_path(u"MinMap/ne");
        auto sw = node->find_from_path(u"MinMap/sw");
        auto se = node->find_from_path(u"MinMap/se");

        return v_s;
    }
}