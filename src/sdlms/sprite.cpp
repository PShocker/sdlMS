#include "sdlms/sprite.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/graphics.hpp"
#include "sprite.hpp"

Sprite::Sprite(std::u16string url,
               const std::vector<uint8_t> &raw_data,
               SDL_FRect rect,
               unsigned int format, int flip,
               int delay, int a0, int a1) : _url(url), _rect(rect),
                                            _format(format), _flip(flip),
                                            _raw_data(raw_data),
                                            _delay(delay), _a0(a0), _a1(a1)
{
    if (texture_map.contains(url))
    {
        // 从缓存中获取
        _texture = texture_map[url];
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
            _texture = SDL_CreateTexture(Graphics::current()->getRenderer(), format, SDL_TEXTUREACCESS_STATIC, rect.w, rect.h);
            SDL_UpdateTexture(_texture, NULL, pixel.data(), rect.w * sizeof(Uint16));
            SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
            break;
        }
        case 2:
        {
            pixel = raw_data;
            format = SDL_PIXELFORMAT_ARGB8888;
            _texture = SDL_CreateTexture(Graphics::current()->getRenderer(), format, SDL_TEXTUREACCESS_STATIC, rect.w, rect.h);
            SDL_UpdateTexture(_texture, NULL, pixel.data(), rect.w * sizeof(Uint32));
            SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
            break;
        }
        case 517: // rgb565压缩缩略图
        {
            pixel.resize(rect.w * rect.h * 2, 0);
            int lineIndex = 0;
            for (int j0 = 0, j1 = rect.h / 16; j0 < j1; j0++)
            {
                int dstIndex = lineIndex;
                for (int i0 = 0, i1 = rect.w / 16; i0 < i1; i0++)
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
                    for (int m = 0; m < rect.w * 2; m++)
                    {
                        pixel[dstIndex + m] = pixel[lineIndex + m];
                    }
                    dstIndex += rect.w * 2;
                }
                lineIndex += rect.w * 32;
            }

            format = SDL_PIXELFORMAT_RGB565;
            _texture = SDL_CreateTexture(Graphics::current()->getRenderer(), format, SDL_TEXTUREACCESS_STATIC, rect.w, rect.h);
            SDL_UpdateTexture(_texture, NULL, pixel.data(), rect.w * sizeof(Uint16));
            break;
        }
        default:
        {
            break;
        }
        }
        texture_map[url] = _texture;
    }
}

Sprite::Sprite(SDL_Texture *texture, SDL_FRect rect, int flip) : _texture(texture), _rect(rect), _flip(flip)
{
}

void Sprite::draw()
{
    auto camera = Camera::current();
    auto graphics = Graphics::current();
    auto fr = rect();
    fr.x -= camera->_viewport.x;
    fr.y -= camera->_viewport.y;
    if (_flip > 0) // 翻转
    {
        graphics->blitSurfaceEx(_texture, NULL, &fr, 0, 0, SDL_FLIP_HORIZONTAL);
    }
    else
    {
        graphics->blitSurface(_texture, NULL, &fr);
    }
}

//
void Sprite::draw(Point<float> position)
{
    auto camera = Camera::current();
    auto graphics = Graphics::current();
    auto fr = rect();
    fr.x = fr.x - camera->_viewport.x + position.x();
    fr.y = fr.y - camera->_viewport.y + position.y();
    if (_flip > 0) // 翻转
    {
        graphics->blitSurfaceEx(_texture, NULL, &fr, 0, 0, SDL_FLIP_HORIZONTAL);
    }
    else
    {
        graphics->blitSurface(_texture, NULL, &fr);
    }
}

// 不随摄像机移动的精灵图
void Sprite::draw_static(Point<float> position)
{
    auto camera = Camera::current();
    auto graphics = Graphics::current();
    auto fr = rect();
    fr.x = fr.x + position.x();
    fr.y = fr.y + position.y();
    if (_flip > 0) // 翻转
    {
        graphics->blitSurfaceEx(_texture, NULL, &fr, 0, 0, SDL_FLIP_HORIZONTAL);
    }
    else
    {
        graphics->blitSurface(_texture, NULL, &fr);
    }
}

// 不随摄像机移动的精灵图
void Sprite::draw_static()
{
    auto camera = Camera::current();
    auto graphics = Graphics::current();
    auto fr = rect();
    if (_flip > 0) // 翻转
    {
        graphics->blitSurfaceEx(_texture, NULL, &fr, 0, 0, SDL_FLIP_HORIZONTAL);
    }
    else
    {
        graphics->blitSurface(_texture, NULL, &fr);
    }
}

SDL_FRect Sprite::rect()
{
    return _rect;
}

Sprite Sprite::load_sprite(wz::Node *node, int x, int y, int flip)
{

    if (node->type == wz::Type::UOL)
    {
        node = dynamic_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }

    auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node);
    auto raw_data = canvas->get_raw_data();
    auto height = canvas->get().height;
    auto width = canvas->get().width;

    auto format = canvas->get().format + canvas->get().format2;

    auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));

    auto ox = 0;
    auto oy = 0;

    if (o != nullptr)
    {
        ox = o->get().x;
        oy = o->get().y;
    }

    SDL_FRect rect{(float)x - ox, (float)y - oy, (float)width, (float)height};

    auto delay = 100;

    auto d = node->get_child(u"delay");
    if (d != nullptr)
    {
        if (d->type == wz::Type::String)
        {
            auto delay_str = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"delay"))->get();
            delay = std::stoi(std::string{delay_str.begin(), delay_str.end()});
        }
        else if (d->type == wz::Type::Int)
        {
            delay = dynamic_cast<wz::Property<int> *>(node->get_child(u"delay"))->get();
        }
    }

    auto a0 = 255;
    auto a1 = 255;
    if (canvas->get_child(u"a0") != nullptr && canvas->get_child(u"a1") != nullptr)
    {
        a0 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a0"))->get();
        a1 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a1"))->get();
    }

    Sprite sprite(node->path, raw_data, rect, (int)format, flip, delay, a0, a1);

    return sprite;
}

Sprite Sprite::load_sprite(wz::Node *node, Point<int32_t> p, int flip)
{
    return load_sprite(node, p.x(), p.y(), flip);
}
