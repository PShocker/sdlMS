#include "util/sprite_util.hpp"

namespace util
{
    Sprite SpriteUtil::load_sprite(wz::Node *node, int x, int y, int flip)
    {
        if (node->type == wz::Type::UOL)
        {
            node = dynamic_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
        }

        auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node);
        auto raw_data = canvas->get_raw_data();
        auto height = canvas->get().height;
        auto width = canvas->get().width;

        auto format = canvas->get().format + canvas->get().format2;

        auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));

        auto ox = 0;
        auto oy = 0;

        if (o != nullptr)
        {
            ox = o->get().x;
            oy = o->get().y;
        }

        SDL_FRect rect{(float)x - ox, (float)y - oy, (float)width, (float)height};

        auto delay = 100;
        auto d = dynamic_cast<wz::Property<int> *>(node->get_child(u"delay"));

        if (d != nullptr)
        {
            delay = d->get();
        }

        auto a0 = 255;
        auto a1 = 255;
        if (canvas->get_child(u"a0") != NULL && canvas->get_child(u"a1") != NULL)
        {
            a0 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a0"))->get();
            a1 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a1"))->get();
        }

        Sprite sprite(node->path, raw_data, rect, (int)format, flip, delay, a0, a1);

        return sprite;
    }

    Sprite SpriteUtil::load_sprite(wz::Node *node, Point<int32_t> p, int flip)
    {
        return load_sprite(node, p.x(), p.y(), flip);
    }

    AnimatedSprite SpriteUtil::load_animated_sprite(wz::Node *node, int x, int y, int flip)
    {
        std::vector<Sprite> v_sprite;
        for (auto it : node->get_children())
        {
            wz::Property<wz::WzCanvas> *canvas;
            if (it.second[0]->type == wz::Type::UOL)
            {
                auto uol = dynamic_cast<wz::Property<wz::WzUOL> *>(it.second[0]);
                canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(uol->get_uol());
            }
            else if (it.second[0]->type == wz::Type::Canvas)
            {
                canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(it.second[0]);
            }
            else
            {
                continue;
            }

            Sprite sprite = load_sprite(canvas, x, y, flip);

            v_sprite.push_back(sprite);
        }
        return AnimatedSprite(v_sprite, v_sprite.size());
    }

    EventSprite *SpriteUtil::load_event_sprite(std::map<std::u16string, uint8_t> event_map, wz::Node *node, int x, int y)
    {
        std::unordered_map<uint8_t, DynamicSprite> eventsprite;

        auto func = [this, x, y, &eventsprite](std::u16string str, uint8_t event, wz::Node *node) -> void
        {
            node = node->find_from_path(str);
            if (node != nullptr)
            {
                auto animatedsprite = load_animated_sprite(node, x, y);
                auto dynamicsprite = DynamicSprite(animatedsprite);
                eventsprite.emplace(event, dynamicsprite);
            }
        };

        for (auto &it : event_map)
        {
            func(it.first, it.second, node);
        }

        return new EventSprite(eventsprite);
    }

}
