#include "sdlms/frame.hpp"
#include "util/sprite_util.hpp"

Frame::Frame(wz::Node *node)
{
    auto _sprite_util = util::SpriteUtil::current();

    center = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"c")));
    east = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e")));
    northeast = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"ne")));
    north = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"n")));
    northwest = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"nw")));
    west = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w")));
    southwest = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"sw")));
    south = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"s")));
    southeast = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"se")));

    xtile = std::max<int16_t>(north._rect.w, 1);
    ytile = std::max<int16_t>(west._rect.h, 1);
}
void Frame::draw(Point<float> position, int16_t rwidth, int16_t rheight)
{
    int16_t numhor = rwidth / xtile + 2;
    int16_t numver = rheight / ytile;
    int16_t width = numhor * xtile;
    int16_t height = numver * ytile;
    int16_t left = position.x() - width / 2;
    int16_t top = position.y() - height;
    int16_t right = left + width;
    int16_t bottom = top + height;

    northwest.draw(Point<float>(left, top));
    southwest.draw(Point<float>(left, bottom));

    for (int16_t x = left; x < right; x += xtile)
    {
        north.draw(Point<float>(x, top));
        south.draw(Point<float>(x, bottom));
    }

    for (int16_t y = top; y < bottom; y += ytile)
    {
        west.draw(Point<float>(left, y));
        east.draw(Point<float>(right, y));
    }

    for (int16_t x = left; x < right; x += xtile)
    {
        for (int16_t y = top; y < bottom; y += ytile)
        {
            center.draw(Point<float>(x, y));
        }
    }

    northeast.draw(Point<float>(right, top));
    southeast.draw(Point<float>(right, bottom));
}