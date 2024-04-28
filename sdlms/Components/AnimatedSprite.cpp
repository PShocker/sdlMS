#include "Animatedsprite.h"
#include <ranges>
#include <algorithm>

AnimatedSprite *AnimatedSprite::load_animated_sprite(wz::Node *node)
{
    // 从map缓存中获取对象
    if (animatedsprite_map.contains(node))
    {
        return animatedsprite_map[node];
    }
    else
    {
        return new AnimatedSprite(node);
    }
}

AnimatedSprite::AnimatedSprite(wz::Node *node)
{
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

        Sprite *sprite = Sprite::load_sprite(canvas);

        sprites.push_back(sprite);
    }
    anim_size = sprites.size();
    anim_index = 0;
    anim_time = 0;

    animatedsprite_map[node] = this;
}


void AnimatedSprite::advance_anim()
{
    if (anim_index == anim_size - 1 && z)
    {

        std::ranges::reverse(sprites);
        anim_index = 1;
    }
    else
    {
        anim_index = (anim_index + 1) % anim_size;
    }
}