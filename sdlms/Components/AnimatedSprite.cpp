#include "AnimatedSprite.h"

AnimatedSprite::AnimatedSprite(wz::Node *node, int alpha)
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

        Sprite *sprite = new Sprite(canvas, alpha);

        sprites.push_back(sprite);
    }
    if (node->get_child(u"zigzag") != nullptr)
    {
        // 如果存在zigzag属性,则认为属于zigzag动画
        z = true;
    }

    anim_size = sprites.size();
    anim_index = 0;
    anim_time = 0;
}

AnimatedSprite::~AnimatedSprite()
{
    for (auto &spr : sprites)
    {
        delete spr;
    }
}