#include "util/sprite_util.hpp"

namespace util
{
    Sprite SpriteUtil::load_sprite(wz::Node *node, int x, int y, int flip)
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

    AnimatedSprite SpriteUtil::load_animated_sprite(wz::Node *node, int x, int y, int flip)
    {
        std::vector<Sprite> v_sprite;
        std::vector<int> v_delay;
        std::vector<std::tuple<int, int>> v_a;
        for (auto it : node->get_children())
        {
            wz::Property<wz::WzCanvas> *canvas;
            if (it.second[0]->type == wz::Type::UOL)
            {
                auto i = dynamic_cast<wz::Property<wz::WzUOL> *>(it.second[0]);
                canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(i->get().uol));
            }
            else if (it.second[0]->type == wz::Type::Canvas)
            {
                canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(it.second[0]);
            }
            else
            {
                continue;
            }
            auto delay = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"delay"));
            v_delay.push_back(delay == nullptr ? 0 : delay->get());

            auto a0 = 255;
            auto a1 = 255;

            if (canvas->get_child(u"a0") != NULL && canvas->get_child(u"a1") != NULL)
            {
                a0 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a0"))->get();
                a1 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a1"))->get();
            }

            v_a.push_back(std::tuple<int, int>(a0, a1));

            Sprite sprite = load_sprite(canvas, x, y, flip);

            v_sprite.push_back(sprite);
        }
        return AnimatedSprite(v_sprite, v_delay, v_sprite.size(), v_a);
    }

    EventSprite *SpriteUtil::load_event_sprite(wz::Node *node, int x, int y)
    {
        std::map<EventSprite::Event, DynamicSprite> eventsprite;

        auto func = [this, x, y, &eventsprite](std::u16string str, EventSprite::Event event, wz::Node *node) -> void
        {
            node = node->find_from_path(str);
            if (node != nullptr)
            {
                auto animatedsprite = load_animated_sprite(node, x, y);
                auto dynamicsprite = DynamicSprite(animatedsprite);
                eventsprite.emplace(event, dynamicsprite);
            }
        };

        for (auto &it : EventSprite::EventMap)
        {
            func(it.first, it.second, node);
        }

        return new EventSprite(eventsprite);
    }

}
