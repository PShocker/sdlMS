#include "Sprite.h"
#include "Core/Window.h"

Sprite::Sprite(wz::Node *node)
{

    if (node->type == wz::Type::UOL)
    {
        node = dynamic_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }

    auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node);
    auto height = canvas->get().height;
    auto width = canvas->get().width;

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

    a0 = 255;
    a1 = 255;
    if (canvas->get_child(u"a0") != nullptr && canvas->get_child(u"a1") != nullptr)
    {
        a0 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a0"))->get();
        a1 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a1"))->get();
    }

    url = node->path;

    if (texture_map.contains(url))
    {
        // 从缓存中获取
        texture = texture_map[url];
    }
    else
    {
        // 图片原始数据,部分格式需要转换
        std::vector<uint8_t> pixel;

        switch (format)
        {
        case 1:
        {
            pixel = raw_data;
            format = SDL_PIXELFORMAT_ARGB4444;
            texture = SDL_CreateTexture(Window::get_renderer(), format, SDL_TEXTUREACCESS_STATIC, width, height);
            SDL_UpdateTexture(texture, NULL, pixel.data(), width * sizeof(Uint16));
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            break;
        }
        case 2:
        {
            pixel = raw_data;
            format = SDL_PIXELFORMAT_ARGB8888;
            texture = SDL_CreateTexture(Window::get_renderer(), format, SDL_TEXTUREACCESS_STATIC, width, height);
            SDL_UpdateTexture(texture, NULL, pixel.data(), width * sizeof(Uint32));
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            break;
        }
        case 517: // rgb565压缩缩略图
        {
            pixel.resize(width * height * 2, 0);
            int lineIndex = 0;
            for (int j0 = 0, j1 = height / 16; j0 < j1; j0++)
            {
                int dstIndex = lineIndex;
                for (int i0 = 0, i1 = width / 16; i0 < i1; i0++)
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
                    for (int m = 0; m < width * 2; m++)
                    {
                        pixel[dstIndex + m] = pixel[lineIndex + m];
                    }
                    dstIndex += width * 2;
                }
                lineIndex += width * 32;
            }

            format = SDL_PIXELFORMAT_RGB565;
            texture = SDL_CreateTexture(Window::get_renderer(), format, SDL_TEXTUREACCESS_STATIC, width, height);
            SDL_UpdateTexture(texture, NULL, pixel.data(), width * sizeof(Uint16));
            break;
        }
        default:
        {
            break;
        }
        }
        texture_map[url] = texture;
    }
}

Sprite::Sprite(SDL_Texture *texture) : texture(texture)
{
}
