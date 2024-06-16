#include "NameTag.h"
#include "Components/Sprite.h"

NameTag::NameTag(int width, int height)
{
    auto renderer = Window::get_renderer();
    Uint8 alpha = 178;
    Uint32 color = (alpha << 24) | (0 << 16) | (0 << 8) | 0; // ARGB格式，这里设置为纯黑色，透明度为178
    std::vector<Uint32> pixel(width * height);
    for (int i = 0; i < width * height; ++i)
    {
        pixel[i] = color;
    }
    auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
    SDL_UpdateTexture(texture, NULL, pixel.data(), width * sizeof(Uint32));
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    Sprite *spr = new Sprite(texture, width, height);
    add_component(spr);
}

NameTag::~NameTag()
{
    if (get_component<Sprite>() != nullptr)
    {
        auto spr = get_component<Sprite>();
        delete spr;
    }
}
