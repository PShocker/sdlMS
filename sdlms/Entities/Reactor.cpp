#include "Reactor.h"
#include "entt/entt.hpp"
#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"
#include "FootHold.h"

void load_reactor(wz::Node *node)
{
    auto ent = World::registry->create();

    auto id = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"id"))->get();
    auto url = id + u".img";
    auto link = id;

    if (auto r = Wz::Reactor->get_root()->find_from_path(url))
    {
        if (r->find_from_path(u"info/link"))
        {
            link = dynamic_cast<wz::Property<wz::wzstring> *>(r->find_from_path(u"info/link"))->get();
            r = Wz::Reactor->get_root()->find_from_path(link + u".img");
        }
        auto reactor = &World::registry->emplace<Reactor>(ent);

        // 从第0帧顺序读
        for (int i = 0; i < r->children_count(); i++)
        {
            auto it = r->find_from_path(std::to_string(i));
            if (it == nullptr)
            {
                break;
            }
            AnimatedSprite hit;
            if (it->find_from_path(u"hit"))
            {
                hit = AnimatedSprite(it->find_from_path(u"hit"));
            }
            std::vector<std::pair<int, int>> event;
            if (it->get_child(u"event"))
            {
                for (auto &[k, v] : it->get_child(u"event")->get_children())
                {
                    if (v[0]->type == wz::Type::SubProperty)
                    {
                        auto type = dynamic_cast<wz::Property<int> *>(v[0]->get_child(u"type"))->get();
                        auto state = dynamic_cast<wz::Property<int> *>(v[0]->get_child(u"state"))->get();
                        event.push_back({type, state});
                    }
                }
            }
            reactor->a[i] = {AnimatedSprite(it), hit, event};
        }

        auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
        auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();
        auto reactorTime = dynamic_cast<wz::Property<int> *>(node->get_child(u"reactorTime"))->get();
        auto f = dynamic_cast<wz::Property<int> *>(node->get_child(u"f"))->get();

        // 计算layer
        auto foo = fhs;
        int layer = 0;
        int top_y = INT32_MAX;
        for (auto &[key, val] : foo)
        {
            if (val->l <= x && val->r >= x && y >= val->t && y <= val->b)
            {
                auto fh_y = val->get_y(x).value();
                if (top_y >= fh_y)
                {
                    top_y = fh_y;
                    layer = val->page;
                }
            }
        }
        World::registry->emplace<Transform>(ent, (float)x, (float)y, layer * LAYER_Z + REACTOR_Z, f);

        size_t first_non_zero = link.find_first_not_of(u'0');
        // 如果找到了非零字符，则从该位置开始截取字符串
        if (first_non_zero != std::string::npos)
        {
            link = link.substr(first_non_zero);
        }
        node = Wz::Sound->get_root()->find_from_path(u"Reactor.img/" + link);
        if (node != nullptr)
        {
            for (auto &[key, val] : node->get_children())
            {
                if (val[0]->get_child(u"Hit"))
                {
                    auto sou = Sound::Wrap::load(val[0]->get_child(u"Hit"));
                    auto i = std::stoi(std::string{key.begin(), key.end()});
                    reactor->sounds[i] = sou;
                }
            }
        }
    }
}