#include "Sprite.h"

#include "wz/Property.hpp"
#include <SDL3/SDL.h>

#include "Core/Core.h"

static std::unordered_map<wz::Node *, SpriteWarp *> cache;

SpriteWarp *SpriteWarp::load(wz::Node *node, int alpha, bool caches, SDL_TextureAccess access)
{
    if (caches == false)
    {
        return new SpriteWarp(node, alpha, access);
    }
    if (cache.contains(node))
    {
        return cache[node];
    }
    else
    {
        SpriteWarp *sprw = new SpriteWarp(node, alpha, access);
        cache[node] = sprw;
        return sprw;
    }
}

SpriteWarp::SpriteWarp(wz::Node *node, int alpha, SDL_TextureAccess access)
{
    if (node->type == wz::Type::UOL)
    {
        node = dynamic_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }
    n = node;

    auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node);
    auto h = canvas->get().height;
    auto w = canvas->get().width;

    auto raw_data = canvas->get_raw_data();
    auto format = canvas->get().format + canvas->get().format2;

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

    // 图片原始数据,部分格式需要转换
    std::vector<uint8_t> pixel;
    switch (format)
    {
    case 1:
    {
        pixel = raw_data;
        texture = SDL_CreateTexture(Window::renderer, SDL_PIXELFORMAT_ARGB4444, access, w, h);
        SDL_UpdateTexture(texture, NULL, pixel.data(), w * sizeof(Uint16));
        break;
    }
    case 2:
    {
        pixel = raw_data;
        texture = SDL_CreateTexture(Window::renderer, SDL_PIXELFORMAT_ARGB8888, access, w, h);
        SDL_UpdateTexture(texture, NULL, pixel.data(), w * sizeof(Uint32));
        break;
    }
    case 517: // rgb565压缩缩略图
    {
        pixel.resize(w * h * 2, 0);
        int lineIndex = 0;
        for (int j0 = 0, j1 = h / 16; j0 < j1; j0++)
        {
            int dstIndex = lineIndex;
            for (int i0 = 0, i1 = h / 16; i0 < i1; i0++)
            {
                int idx = (i0 + j0 * i1) * 2;
                unsigned char b0 = raw_data[idx];
                unsigned char b1 = raw_data[idx + 1];
                for (int k = 0; k < 16; k++)
                {
                    pixel[dstIndex++] = b0;
                    pixel[dstIndex++] = b1;
                }
            }
            for (int k = 1; k < 16; k++)
            {
                for (int m = 0; m < w * 2; m++)
                {
                    pixel[dstIndex + m] = pixel[lineIndex + m];
                }
                dstIndex += w * 2;
            }
            lineIndex += w * 32;
        }
        texture = SDL_CreateTexture(Window::renderer, SDL_PIXELFORMAT_RGB565, access, w, h);
        SDL_UpdateTexture(texture, NULL, pixel.data(), w * sizeof(Uint16));
        break;
    }
    default:
    {
        break;
    }
    }
}

Sprite::Sprite(wz::Node *node, int alpha)
{
    spr = SpriteWarp::load(node, alpha);
}

Sprite::Sprite(SpriteWarp *spr) : spr(spr)
{
}

void SpriteWarp::clean_up()
{
    for (auto &[key, val] : cache)
    {
        SDL_DestroyTexture(val->texture);
        delete val;
    }
    cache.clear();
}