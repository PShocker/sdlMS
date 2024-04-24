#include "sdlms/npc.hpp"
#include "util/map_util.hpp"
#include "sdlms/foothold.hpp"
#include "util/wz_util.hpp"

Npc::Npc(EventSprite eventsprite, std::u16string id, int fh) : _es(eventsprite),
                                                               _id(id),
                                                               _fh(fh),
                                                               _gen(std::random_device()()),
                                                               _dist(0, 0)

{
    for (const auto &[event, _] : eventsprite._eventsprite)
    {
        _v_event.push_back(event);
    }
    _dist.param(std::uniform_int_distribution<uint8_t>::param_type(0, _v_event.size() - 1));
}

void Npc::draw()
{
    _es.draw(_event);
    _name.draw();
    _func.draw();

    // frame.draw({_eventsprite.at(_event).rect().x, _eventsprite.at(_event).rect().y}, 100, 100);
}

void Npc::update(int elapsedTime)
{
    // auto end = _eventsprite.at(_event).update(elapsedTime);
    auto end = _es.update(_event, elapsedTime);
    if (end)
    {
        // 状态切换
        _event = _v_event[_dist(_gen)];
    }
}

void Npc::updates(int i, int elapsedTime)
{
    for (auto &it : _npc[i])
    {
        // 更新帧状态
        it.update(elapsedTime);
    }
}

void Npc::draws(int i)
{
    for (auto &it : _npc[i])
    {
        it.draw();
    }
}

void Npc::load_npc(int mapId)
{
    auto node = util::MapUtil::current()->load_map_node(mapId);
    auto fhs = FootHold::load_foothold(mapId);

    node = node->get_child(u"life");
    if (node != nullptr)
    {
        for (auto it : node->get_children())
        {
            auto type = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"type"))->get();

            if (type == u"n")
            {
                auto npc_id = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"id"))->get();
                auto x = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"x"))->get();
                auto y = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"cy"))->get();
                auto fh = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"fh"))->get();
                // 从fh获取layer
                auto layer = fhs.at(fh)._page;
                auto npc_node = util::WzUtil::current()->Npc->get_root()->find_from_path(npc_id + u".img");

                if (npc_node != nullptr)
                {
                    auto id_pos = npc_id.find_first_not_of(u'0');
                    // 去掉npc_id中前缀的0
                    npc_id = npc_id.substr(id_pos);
                    auto npc_info = util::StringUtil::current()->load_npc_info(npc_id);
                    auto name = npc_info[u"name"];
                    auto func = npc_info[u"func"];
                    auto eventsprite = EventSprite::load_event_sprite(Npc::EventMap, npc_node, x, y);

                    auto npc = Npc(eventsprite, npc_id, fh);
                    npc._name = util::FreeTypeUtil::current()->load_npc_str(name, 14);
                    npc._func = util::FreeTypeUtil::current()->load_npc_str(func, 14);

                    npc._name._rect.x = x - npc._name._rect.w / 2;
                    npc._name._rect.y = y + 4;

                    npc._func._rect.x = x - npc._func._rect.w / 2;
                    npc._func._rect.y = y + 20;
                    auto ballon = util::WzUtil::current()->UI->get_root()->find_from_path(u"ChatBalloon.img/0");
                    npc.frame = Frame(ballon);
                    _npc[layer].push_back(npc);
                }
            }
        }
    }
}