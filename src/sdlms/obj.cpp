#include "sdlms/obj.hpp"
#include "util/wz_util.hpp"
#include "util/map_util.hpp"

Obj::Obj(int id,
         AnimatedSprite animatedsprite,
         int layer,
         int z, std::u16string url) : _id(id),
                                      AnimatedSprite(animatedsprite),
                                      _layer(layer), _z(z),
                                      _url(url)
{
}

void Obj::draws(std::vector<Obj> &objs)
{
    for (auto &it : objs)
    {
        it.draw();
    }
}

void Obj::updates(std::vector<Obj> &objs, int elapsedTime)
{
    for (auto &it : objs)
    {
        // 更新帧状态
        it.update(elapsedTime);
    }
}

std::array<std::vector<Obj>, 8> Obj::load_obj(int mapId)
{

    std::array<std::vector<Obj>, 8> obj;
    for (size_t i = 0; i < 8; i++)
    {
        obj[i] = load_obj(util::MapUtil::current()->load_map_node(mapId), i);
    }
    return obj;
}

std::vector<Obj> Obj::load_obj(wz::Node *node, int i)
{

    std::vector<Obj> obj;
    node = node->get_child(std::to_string(i))->get_child(u"obj");

    auto _map_node = util::WzUtil::current()->Map->get_root();

    for (auto it : node->get_children())
    {
        std::vector<Sprite> v_sprite;
        std::vector<int> v_delay;
        std::vector<std::tuple<int, int>> v_a;
        auto oS = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"oS"))->get();
        auto l0 = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"l0"))->get();
        auto l1 = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"l1"))->get();
        auto l2 = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"l2"))->get();
        auto url = u"Obj/" + oS + u".img/" + l0 + u"/" + l1 + u"/" + l2;

        auto x = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"x"))->get();
        auto y = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"y"))->get();
        auto z = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"z"))->get();

        auto flip = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"f"))->get();

        auto animatedsprite = AnimatedSprite::load_animated_sprite(_map_node->find_from_path(url), x, y, flip);

        auto id = std::stoi(std::string{it.first.begin(), it.first.end()});

        Obj o(id, animatedsprite, i, z, url);
        obj.push_back(o);
    }
    std::ranges::sort(obj, [](const Obj a, const Obj b)
                      { return a._z == b._z ? (a._id < b._id) : (a._z < b._z); });
    return obj;
}
