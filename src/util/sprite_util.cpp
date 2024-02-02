#include "util/sprite_util.hpp"

namespace util
{
    Sprite SpriteUtil::load_sprite(wz::Node *node, int x, int y)
    {
        auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node);
        auto raw_data = canvas->get_raw_data();
        auto height = canvas->get().height;
        auto width = canvas->get().width;

        auto format = canvas->get().format;

        auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));

        auto ox = 0;
        auto oy = 0;

        if (o != nullptr)
        {
            ox = o->get().x;
            oy = o->get().y;
        }

        SDL_FRect rect{(float)x - ox, (float)y - oy, (float)width, (float)height};

        Sprite sprite(raw_data, rect, (int)format);

        return sprite;
    }
}
