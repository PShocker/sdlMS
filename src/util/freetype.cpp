#include "sdlms/graphics.hpp"
#include "util/freetype.hpp"

namespace util
{
    FreeType::FreeType()
    {
        _renderer = Graphics::current()->getRenderer();

        _library = new FT_Library{};
        FT_Init_FreeType(_library);
        // 加载字体文件
        _face = new FT_Face{};
        FT_New_Face(*_library, (filename_prefix + "font.ttf").c_str(), 0, _face);
        FT_Set_Pixel_Sizes(*_face, 0, 48);
    }

    SDL_Texture *FreeType::load_str(const std::string &s)
    {

        FT_Load_Char(*_face, s.c_str()[0], FT_LOAD_RENDER);

        FT_GlyphSlot g = (*_face)->glyph;
        // 创建SDL纹理并将位图数据复制到纹理中
        SDL_Texture *texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, g->bitmap.width, g->bitmap.rows);
        SDL_UpdateTexture(texture, NULL, g->bitmap.buffer, g->bitmap.pitch);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        return texture;
    }
}