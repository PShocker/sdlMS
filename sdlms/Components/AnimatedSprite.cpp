module;

#include <vector>
#include "wz/Property.hpp"

module components;

import :sprite;

static std::unordered_map<wz::Node *, AnimatedSpriteWarp *> cache;

AnimatedSpriteWarp *AnimatedSpriteWarp::load(wz::Node *node, int alpha)
{
    if (cache.contains(node))
    {
        return cache[node];
    }
    else
    {
        AnimatedSpriteWarp *aspr = new AnimatedSpriteWarp(node, alpha);
        cache[node] = aspr;
        return aspr;
    }
}

AnimatedSpriteWarp::AnimatedSpriteWarp(wz::Node *node, int alpha)
{
    if (node->type == wz::Type::UOL)
    {
        node = dynamic_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }

    // 从第0帧顺序读
    for (int i = 0; i < node->children_count(); i++)
    {
        auto it = node->get_child(std::to_string(i));
        if (it == nullptr)
        {
            // 如果发现没读取到,说明已经读完,则退出读取
            break;
        }
        wz::Property<wz::WzCanvas> *canvas;
        if (it->type == wz::Type::UOL)
        {
            auto uol = dynamic_cast<wz::Property<wz::WzUOL> *>(it);
            canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(uol->get_uol());
        }
        else if (it->type == wz::Type::Canvas)
        {
            canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(it);
        }
        else
        {
            continue;
        }
        sprites.push_back(SpriteWarp::load(canvas, alpha));
    }
    if (node->get_child(u"zigzag") != nullptr)
    {
        // 如果存在zigzag属性,则认为属于zigzag动画
        z = true;
    }
}

AnimatedSprite::AnimatedSprite(wz::Node *node, int alpha)
{
    aspr = AnimatedSpriteWarp::load(node, alpha);
    anim_size = aspr->sprites.size();
    anim_index = 0;
    anim_time = 0;
}

AnimatedSprite::AnimatedSprite(AnimatedSpriteWarp *aspr) : aspr(aspr)
{
    anim_size = aspr->sprites.size();
}
