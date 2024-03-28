#include "sdlms/sprite.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/graphics.hpp"

Sprite::Sprite(const std::vector<uint8_t> &raw_data,
               SDL_FRect rect,
               unsigned int format, int flip) : _rect(rect),
                                                _format(format), _flip(flip),
                                                _raw_data(raw_data)
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

void Sprite::_draw()
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
