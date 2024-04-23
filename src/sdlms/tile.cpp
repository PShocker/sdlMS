#include "sdlms/tile.hpp"
#include "util/wz_util.hpp"
#include "util/map_util.hpp"

Tile::Tile(Sprite sprite, int layer, int z) : Sprite(sprite),
                                              _layer(layer),
                                              _z(z)
{
}

void Tile::draws(std::vector<Tile> &tiles)
{
    for (auto &it : tiles)
    {
        it.draw();
    }
}

std::array<std::vector<Tile>, 8> Tile::load_tile(int mapId)
{
    std::array<std::vector<Tile>, 8> tile;
    for (size_t i = 0; i < 8; i++)
    {
        tile[i] = load_tile(util::MapUtil::current()->load_map_node(mapId), i);
    }
    return tile;
}
std::vector<Tile> Tile::load_tile(wz::Node *node, int i)
{
    std::vector<Tile> tile;
    node = node->get_child(std::to_string(i));
    auto tS = node->get_child(u"info")->get_child(u"tS");

    auto _map_node = util::WzUtil::current()->Map->get_root();

    if (tS != nullptr)
    {
        for (auto it : node->get_child(u"tile")->get_children())
        {
            auto u = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"u"))->get();
            auto no = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"no"))->get();
            auto x = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"x"))->get();
            auto y = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"y"))->get();

            auto no_str = std::to_string(no);

            auto url = u"Tile/" + dynamic_cast<wz::Property<wz::wzstring> *>(tS)->get() + u".img/" + u + u"/" + std::u16string{no_str.begin(), no_str.end()};

            auto tn = _map_node->find_from_path(url);

            Sprite sprite = Sprite::load_sprite(tn, x, y);

            auto z = dynamic_cast<wz::Property<int> *>(tn->get_child(u"z"))->get();

            Tile t(sprite, i, z);

            tile.push_back(t);
        }
    }
    std::ranges::sort(tile, [](const Tile a, const Tile b)
                      { return a._z < b._z; });
    return tile;
}
