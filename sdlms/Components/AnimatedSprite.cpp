#include "Animatedsprite.h"
#include <ranges>
#include <algorithm>

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

        Sprite *sprite = new Sprite(canvas);

        sprites.push_back(sprite);
    }
    frame_size = sprites.size();
}

bool AnimatedSprite::update(int elapsedTime)
{
    bool end = false;
    frame_time += elapsedTime;
    // 判断时间是否超过sprite时间
    if (frame_time >= sprites[frame_index]->delay)
    {
        // 判断是否是最后1帧
        if (frame_index == frame_size - 1)
        {
            // 如果zigzag存在,说明动画是1-2-3-2-1形式的倒放
            if (z)
            {
                // 反转vector,形成倒放效果
                std::ranges::reverse(sprites);
                // 切换到倒数第二帧
                frame_index = 1;
            }
            else
            {
                // 如果zigzag不存在,直接从头开始
                frame_index = 0;
            }
            end = true;
            // 表示一个循环结束,该值会EventSprite会使用
        }
        else
        {
            frame_index += 1;
        }
        frame_time = 0;
    }

    // 透明度处理
    auto a0 = sprites[frame_index]->a0;
    auto a1 = sprites[frame_index]->a1;
    if (a0 != a1)
    {
        auto alpha = 255;
        if (a0 <= a1)
        {
            alpha = (float)a0 + (float)(a1 - a0) / (float)sprites[frame_index]->delay * (float)frame_time;
        }
        else
        {
            alpha = (float)a0 - (float)(a0 - a1) / (float)sprites[frame_index]->delay * (float)frame_time;
        }
        SDL_SetTextureAlphaMod(sprites[frame_index]->texture, alpha);
    }

    return end;
}