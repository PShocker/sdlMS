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
        FT_Set_Pixel_Sizes(*_face, 0, 24);
    }

    SDL_Texture *FreeType::load_str(const std::string &s)
    {

        FT_Load_Char(*_face, s.c_str()[0], FT_LOAD_RENDER);

        auto glyph = (*_face)->glyph;
        SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(
            glyph->bitmap.buffer, glyph->bitmap.width, glyph->bitmap.rows,
            8, glyph->bitmap.pitch, SDL_PIXELFORMAT_RGBA32);

        SDL_Texture *texture = SDL_CreateTextureFromSurface(_renderer, surface);
        SDL_FreeSurface(surface);
        return texture;
    }
}