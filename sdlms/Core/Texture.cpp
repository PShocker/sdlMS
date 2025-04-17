#include "Texture.h"
#include "Window.h"

SDL_Texture *Texture::load(wz::Property<wz::WzCanvas> *canvas)
{
    static std::unordered_map<wz::Property<wz::WzCanvas> *, SDL_Texture *> cache;
    if (cache.contains(canvas))
    {
        return cache[canvas];
    }
    else
    {
        auto h = canvas->get().height;
        auto w = canvas->get().width;

        auto raw_data = canvas->get_raw_data();
        auto format = canvas->get().format + canvas->get().format2;
        SDL_Texture *texture = nullptr;

        // 图片原始数据,部分格式需要转换
        std::vector<uint8_t> pixel;
        switch (format)
        {
        case 1:
        {
            pixel = raw_data;
            texture = SDL_CreateTexture(Window::renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STATIC, w, h);
            SDL_UpdateTexture(texture, NULL, pixel.data(), w * sizeof(Uint16));
            break;
        }
        case 2:
        {
            pixel = raw_data;
            texture = SDL_CreateTexture(Window::renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
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
            texture = SDL_CreateTexture(Window::renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STATIC, w, h);
            SDL_UpdateTexture(texture, NULL, pixel.data(), w * sizeof(Uint16));
            break;
        }
        default:
        {
            break;
        }
        }
        cache[canvas] = texture;
        return texture;
    }
}

SDL_Texture *Texture::createBlankTexture(SDL_PixelFormat format, int width, int height)
{

    SDL_Renderer *renderer = Window::renderer;
    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             format,
                                             SDL_TEXTUREACCESS_TARGET,
                                             width, height);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, nullptr);

    return texture;
}
