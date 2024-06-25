#include "Sprite.h"
#include "Core/Window.h"

Sprite::Sprite(wz::Node *node, int alpha)
{
    if (node->type == wz::Type::UOL)
    {
        node = dynamic_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }

    auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node);
    height = canvas->get().height;
    width = canvas->get().width;

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

    if (canvas->get_child(u"a0") != nullptr && canvas->get_child(u"a1") != nullptr)
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

    if (canvas->get_child(u"z") != nullptr)
    {
        if (canvas->get_child(u"z")->type == wz::Type::Int)
        {
            z = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"z"))->get();
        }
        else if (canvas->get_child(u"z")->type == wz::Type::String)
        {
            z = dynamic_cast<wz::Property<wz::wzstring> *>(canvas->get_child(u"z"))->get();
        }
    }

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
}

Sprite::Sprite(wz::Node *node, int w, int h, uint8_t type)
{
    switch (type)
    {
    case NameTag:
    {
        auto w_canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w"));
        auto w_height = w_canvas->get().height;
        auto w_width = w_canvas->get().width;
        auto w_raw_data = w_canvas->get_raw_data();

        auto c_canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"c"));
        auto c_height = c_canvas->get().height;
        auto c_width = c_canvas->get().width;
        auto c_raw_data = c_canvas->get_raw_data();

        auto e_canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e"));
        auto e_height = e_canvas->get().height;
        auto e_width = e_canvas->get().width;
        auto e_raw_data = e_canvas->get_raw_data();

        width = w + w_width + e_width;
        height = std::max(h, w_height);
        texture = SDL_CreateTexture(Window::get_renderer(), SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STATIC, width, height);

        SDL_Rect rect{0, 0, w_width, w_height};
        SDL_UpdateTexture(texture, &rect, w_raw_data.data(), w_width * sizeof(Uint16));

        for (int i = 0; i <= (width - w_width - e_width) / c_width; i++)
        {
            rect = {w_width + i * c_width, 0, c_width, c_height};
            SDL_UpdateTexture(texture, &rect, c_raw_data.data(), c_width * sizeof(Uint16));
        }
        rect = {width - e_width, 0, e_width, e_height};
        SDL_UpdateTexture(texture, &rect, e_raw_data.data(), e_width * sizeof(Uint16));
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        break;
    }
    case ChatBallon:
    {

        break;
    }
    default:
        break;
    }
}

Sprite::Sprite(SDL_Texture *texture, int w, int h) : texture(texture), width(w), height(h)
{
}

Sprite::~Sprite()
{
    SDL_DestroyTexture(texture);
}
