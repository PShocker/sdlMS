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
        FT_New_Face(*_library, (filename_prefix + "font.ttc").c_str(), 0, _face);
        FT_Set_Pixel_Sizes(*_face, 0, 64);
        FT_Select_Charmap(*_face, FT_ENCODING_UNICODE);
    }

    SDL_Texture *FreeType::load_str(const std::wstring &s)
    {

        FT_Load_Glyph(*_face, FT_Get_Char_Index(*_face, s.c_str()[0]), FT_LOAD_RENDER);

        FT_GlyphSlot glyph = (*_face)->glyph;
        // 转换为ARGB8888格式
        unsigned char *data = glyph->bitmap.buffer;
        int width = glyph->bitmap.width;
        int height = glyph->bitmap.rows;

        unsigned char *argbData = new unsigned char[width * height * 4];
        for (int i = 0; i < width * height; ++i)
        {
            argbData[4 * i] = 255;
            argbData[4 * i + 1] = data[i];
            argbData[4 * i + 2] = data[i];
            argbData[4 * i + 3] = data[i];
        }
        // 创建SDL纹理并将位图数据复制到纹理中
        SDL_Texture *texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
        SDL_UpdateTexture(texture, NULL, argbData, width * sizeof(Uint32));
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        delete argbData;

        return texture;
    }
}