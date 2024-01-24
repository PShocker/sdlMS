#include "sdlms/ttf.hpp"

TTF::TTF()
{
    _renderer = Graphics::current()->getRenderer();
    // 加载字体文件
    // 初始化SDL_ttf
    if (TTF_Init() != 0)
    {
        printf("Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        return;
    }
    _font = TTF_OpenFont((std::string(filename_prefix) + std::string("font.ttf")).c_str(), 24);
    if (_font == NULL)
    {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }
}

SDL_Texture *TTF::load_font(const std::string &s)
{
    // 设置文本颜色
    SDL_Color textColor = {255, 255, 255};

    // 创建SDL表面
    SDL_Surface *textSurface = TTF_RenderUTF8_Blended(_font, s.c_str(), textColor);
    if (textSurface == NULL)
    {
        printf("Failed to render text! SDL_ttf Error: %s\n", TTF_GetError());
        return nullptr;
    }

    // 创建SDL纹理
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(_renderer, textSurface);
    if (textTexture == NULL)
    {
        printf("Failed to create texture! SDL Error: %s\n", SDL_GetError());
        return nullptr;
    }
    SDL_FreeSurface(textSurface);
    return textTexture;
}