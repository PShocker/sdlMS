#include "Sprite.h"

#include "wz/Property.hpp"
#include <SDL3/SDL.h>

#include "Core/Core.h"

static std::unordered_map<wz::Node *, SpriteWarp *> cache;

SpriteWarp *SpriteWarp::load(wz::Node *node, int alpha)
{
    if (cache.contains(node))
    {
        return cache[node];
    }
    else
    {
        SpriteWarp *sprw = new SpriteWarp(node, alpha);
        cache[node] = sprw;
        return sprw;
    }
}

SpriteWarp::SpriteWarp(wz::Node *node, int alpha)
{
    if (node->type == wz::Type::UOL)
    {
        node = dynamic_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }
    n = node;

    auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node);

    texture = Texture::load(canvas);

    auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));

    auto ox = 0;
    auto oy = 0;

    if (o != nullptr)
    {
        ox = o->get().x;
        oy = o->get().y;
    }
    origin = {ox, oy};

    delay = 100;

    if (node->get_child(u"delay") != nullptr)
    {
        if (node->get_child(u"delay")->type == wz::Type::String)
        {
            auto delay_str = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"delay"))->get();
            delay = std::stoi(std::string{delay_str.begin(), delay_str.end()});
        }
        else if (node->get_child(u"delay")->type == wz::Type::Int)
        {
            delay = dynamic_cast<wz::Property<int> *>(node->get_child(u"delay"))->get();
        }
    }
    if (canvas->get_child(u"a0") != nullptr)
    {
        if (canvas->get_child(u"a0")->type == wz::Type::Int)
        {
            a0 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a0"))->get();
        }
        else
        {
            auto a0_str = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"a0"))->get();
            a0 = std::stoi(std::string{a0_str.begin(), a0_str.end()});
        }
        a1 = a0;
    }
    if (canvas->get_child(u"a1") != nullptr)
    {
        if (canvas->get_child(u"a1")->type == wz::Type::Int)
        {
            a1 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a1"))->get();
        }
        else
        {
            auto a1_str = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"a1"))->get();
            a1 = std::stoi(std::string{a1_str.begin(), a1_str.end()});
        }
    }
    a0 = a0 * ((float)alpha / 255);
    a1 = a1 * ((float)alpha / 255);
}

Sprite::Sprite(wz::Node *node, int alpha)
{
    sprw = SpriteWarp::load(node, alpha);
}

Sprite::Sprite(SpriteWarp *sprw) : sprw(sprw)
{
}