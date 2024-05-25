#include "NameTag.h"
#include "Components/Sprite.h"

NameTag::NameTag(int width, int height)
{
    auto renderer = Window::get_renderer();
    auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
    // 设置渲染目标为纹理
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    // 填充纹理为黑色
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 178); // 黑色
    SDL_RenderClear(renderer);
    // 恢复默认渲染目标
    SDL_SetRenderTarget(renderer, NULL);
    
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
