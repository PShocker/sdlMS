#include "sdlms/frame.hpp"

Frame::Frame(wz::Node *node)
{
    center = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"c")));
    east = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e")));
    northeast = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"ne")));
    north = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"n")));
    northwest = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"nw")));
    west = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w")));
    southwest = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"sw")));
    south = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"s")));
    southeast = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"se")));

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

void Frame::draw_static(Point<float> position, int16_t rwidth, int16_t rheight)
{
    int16_t numhor = rwidth / xtile + 2;
    int16_t numver = rheight / ytile;
    int16_t width = numhor * xtile;
    int16_t height = numver * ytile;
    int16_t left = position.x() - width / 2;
    int16_t top = position.y() - height;
    int16_t right = left + width;
    int16_t bottom = top + height;

    northwest.draw_static(Point<float>(left, top));
    southwest.draw_static(Point<float>(left, bottom));

    for (int16_t x = left; x < right; x += xtile)
    {
        north.draw_static(Point<float>(x, top));
        south.draw_static(Point<float>(x, bottom));
    }

    for (int16_t y = top; y < bottom; y += ytile)
    {
        west.draw_static(Point<float>(left, y));
        east.draw_static(Point<float>(right, y));
    }

    for (int16_t x = left; x < right; x += xtile)
    {
        for (int16_t y = top; y < bottom; y += ytile)
        {
            center.draw_static(Point<float>(x, y));
        }
    }

    northeast.draw_static(Point<float>(right, top));
    southeast.draw_static(Point<float>(right, bottom));
}