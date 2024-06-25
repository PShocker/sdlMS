#include "NameTag.h"
#include "Components/Sprite.h"
#include "Resource/Wz.h"

NameTag::NameTag(int width, int height)
{
    width += 4;
    height += 6;
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

NameTag::NameTag(int width, int height, const std::u16string &val)
{
    width += 4;
    height += 6;
    Sprite *spr = new Sprite(World::get_world()->get_resource<Wz>().UI->get_root()->find_from_path(u"NameTag.img/" + val),
                             width, height, Sprite::NameTag);
    add_component(spr);
}

int NameTag::get_width()
{
    return get_component<Sprite>()->get_width();
}

int NameTag::get_height()
{
    return get_component<Sprite>()->get_height();
}

NameTag::~NameTag()
{
    auto spr = get_component<Sprite>();
    delete spr;
}
