#include <freetype/ftoutln.h>

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
        FT_New_Face(*_library, (filename_prefix + "DroidSansMono.ttf").c_str(), 0, _face);
        // 设置字体大小
        int fontSize = 12;
        FT_Set_Char_Size(*_face, fontSize * 64, fontSize * 64, 90, 90);
        // FT_Set_Pixel_Sizes(*_face, 0, 48);
    }

    std::tuple<SDL_Texture *, int, int> FreeType::load_str(const std::wstring &s)
    {
        FT_GlyphSlot glyph = (*_face)->glyph;

        // 计算每个字符的位置和大小
        int width = 0;
        int height = 0;
        for (auto &c : s)
        {
            FT_Load_Glyph(*_face, FT_Get_Char_Index(*_face, c), FT_LOAD_RENDER);
            width += glyph->bitmap.width;
            height = glyph->bitmap.rows > height ? glyph->bitmap.rows : height;
        }

        SDL_Texture *texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);

        int offsetX = 0;

        for (auto &c : s)
        {

            FT_Load_Glyph(*_face, FT_Get_Char_Index(*_face, c), FT_LOAD_RENDER);
            SDL_Rect charRect = {offsetX, height - glyph->bitmap.rows, (int)glyph->bitmap.width, (int)glyph->bitmap.rows};
            // 转换为ARGB8888格式
            unsigned char *data = glyph->bitmap.buffer;

            unsigned char *argbData = new unsigned char[glyph->bitmap.width * glyph->bitmap.rows * 4];
            for (int i = 0; i < glyph->bitmap.width * glyph->bitmap.rows; ++i)
            {
                argbData[4 * i] = 0;
                argbData[4 * i + 1] = 0;
                argbData[4 * i + 2] = 0;
                argbData[4 * i + 3] = data[i];
            }
            SDL_UpdateTexture(texture, &charRect, argbData, glyph->bitmap.width * sizeof(Uint32));

            delete argbData;
            offsetX += glyph->bitmap.width;
        }

        // 创建SDL纹理并将位图数据复制到纹理中

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        return {texture, width, height};
    }
}