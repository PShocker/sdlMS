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

Sprite::Sprite(wz::Node *node, int width, int height, uint8_t type)
{
    switch (type)
    {
    case NameTag:
    {
        auto w = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w"));
        auto e = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e"));
        auto c = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"c"));

        width = width + w->get().width + e->get().width;
        height = std::max(height, w->get().height);
        texture = SDL_CreateTexture(Window::get_renderer(), SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STATIC, width, height);

        SDL_Rect rect{0, 0, w->get().width, w->get().height};
        SDL_UpdateTexture(texture, &rect, w->get_raw_data().data(), w->get().width * sizeof(Uint16));

        for (int i = 0; i <= (width - w->get().width - e->get().width) / c->get().width; i++)
        {
            rect = {w->get().width + i * c->get().width, 0, c->get().width, c->get().height};
            SDL_UpdateTexture(texture, &rect, c->get_raw_data().data(), c->get().width * sizeof(Uint16));
        }
        rect = {width - e->get().width, 0, e->get().width, e->get().height};
        SDL_UpdateTexture(texture, &rect, e->get_raw_data().data(), e->get().width * sizeof(Uint16));
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        this->width = width;
        this->height = height;
        break;
    }
    case ChatBallon:
    {
        auto c = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"c"));
        auto e = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e"));
        auto ne = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"ne"));
        auto n = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"n"));
        auto nw = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"nw"));
        auto w = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w"));
        auto sw = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"sw"));
        auto s = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"s"));
        auto se = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"se"));
        auto arrow = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"arrow"));

        width += nw->get().width + ne->get().width;

        height += nw->get().height + sw->get().height;
        auto line = std::ceil((float)height / (float)c->get().height);
        height = line * c->get().height;

        texture = SDL_CreateTexture(Window::get_renderer(), SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STATIC, width, height + 8);
        SDL_Rect rect;

        for (int x = 0; x <= width; x += c->get().width)
        {
            for (int y = 0; y <= height - c->get().height; y += c->get().height)
            {
                rect = {x, y, c->get().width, c->get().height};
                SDL_UpdateTexture(texture, &rect, c->get_raw_data().data(), c->get().width * sizeof(Uint16));
            }
        }

        for (int i = nw->get().height; i <= height - w->get().height; i += w->get().height)
        {
            rect = {0, i, w->get().width, w->get().height};
            SDL_UpdateTexture(texture, &rect, w->get_raw_data().data(), w->get().width * sizeof(Uint16));

            rect = {width - e->get().width, i, e->get().width, e->get().height};
            SDL_UpdateTexture(texture, &rect, e->get_raw_data().data(), e->get().width * sizeof(Uint16));
        }

        for (int i = 0; i <= width; i += n->get().width)
        {
            rect = {i, 0, n->get().width, n->get().height};
            SDL_UpdateTexture(texture, &rect, n->get_raw_data().data(), n->get().width * sizeof(Uint16));

            rect = {i, height - s->get().height, s->get().width, s->get().height};
            SDL_UpdateTexture(texture, &rect, s->get_raw_data().data(), s->get().width * sizeof(Uint16));
        }

        rect = {0, nw->get().height, w->get().width, w->get().height};
        SDL_UpdateTexture(texture, &rect, w->get_raw_data().data(), w->get().width * sizeof(Uint16));

        rect = {0, height - sw->get().height - w->get().height, w->get().width, w->get().height};
        SDL_UpdateTexture(texture, &rect, w->get_raw_data().data(), w->get().width * sizeof(Uint16));

        rect = {width - e->get().width, nw->get().height, e->get().width, e->get().height};
        SDL_UpdateTexture(texture, &rect, e->get_raw_data().data(), e->get().width * sizeof(Uint16));

        rect = {width - e->get().width, height - sw->get().height - e->get().height, e->get().width, e->get().height};
        SDL_UpdateTexture(texture, &rect, e->get_raw_data().data(), e->get().width * sizeof(Uint16));

        rect = {0, 0, nw->get().width, nw->get().height};
        SDL_UpdateTexture(texture, &rect, nw->get_raw_data().data(), nw->get().width * sizeof(Uint16));

        rect = {0, height - sw->get().height, sw->get().width, sw->get().height};
        SDL_UpdateTexture(texture, &rect, sw->get_raw_data().data(), sw->get().width * sizeof(Uint16));

        rect = {width - ne->get().width, 0, ne->get().width, ne->get().height};
        SDL_UpdateTexture(texture, &rect, ne->get_raw_data().data(), ne->get().width * sizeof(Uint16));

        rect = {width - se->get().width, height - se->get().height, se->get().width, se->get().height};
        SDL_UpdateTexture(texture, &rect, se->get_raw_data().data(), se->get().width * sizeof(Uint16));

        rect = {width / 2 - arrow->get().width / 2, height - arrow->get().height + 8, arrow->get().width - 1, arrow->get().height};
        SDL_UpdateTexture(texture, &rect, arrow->get_raw_data().data() + 2, arrow->get().width * sizeof(Uint16));

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        this->width = width;
        this->height = height;
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
