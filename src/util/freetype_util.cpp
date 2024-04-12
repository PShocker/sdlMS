#include <freetype/ftoutln.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include "sdlms/graphics.hpp"
#include "util/freetype_util.hpp"

namespace util
{
    FreeTypeUtil::FreeTypeUtil()
    {
        _renderer = Graphics::current()->getRenderer();

        _library = new FT_Library{};
        FT_Init_FreeType(_library);
        // 加载字体文件
        _face = new FT_Face{};
        FT_New_Face(*_library, (filename_prefix + "/simsun.ttc").c_str(), 0, _face);
        // 设置字体大小
        int fontSize = 16;
        // FT_Set_Char_Size(*_face, fontSize * 64, fontSize * 64, 72, 72);
        FT_Set_Pixel_Sizes(*_face, 0, fontSize);
    }

    std::tuple<SDL_Texture *, int, int> FreeTypeUtil::load_str(const std::u16string &s)
    {
        FT_GlyphSlot glyph_slot = (*_face)->glyph;

        // 计算每个字符的位置和大小
        int width = 0;
        int height = 0;
        for (auto &c : s)
        {
            FT_Load_Glyph(*_face, FT_Get_Char_Index(*_face, c), FT_LOAD_DEFAULT);

            FT_Render_Glyph(glyph_slot, FT_RENDER_MODE_NORMAL);

            width += glyph_slot->bitmap.width;
            height = glyph_slot->bitmap.rows > height ? glyph_slot->bitmap.rows : height;
        }

        SDL_Texture *texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);

        int offsetX = 0;

        for (auto &c : s)
        {
            FT_Load_Glyph(*_face, FT_Get_Char_Index(*_face, c), FT_LOAD_DEFAULT);

            FT_Render_Glyph(glyph_slot, FT_RENDER_MODE_NORMAL);

            auto bitmap = glyph_slot->bitmap;
            // 默认底部对齐
            SDL_Rect charRect = {offsetX, (height - glyph_slot->bitmap.rows), (int)glyph_slot->bitmap.width, (int)glyph_slot->bitmap.rows};
            // 特殊符号居中
            if (c==u'<'||c==u'>')
            {
                charRect.y=(height - glyph_slot->bitmap.rows)/2;
            }
            // 转换为ARGB8888格式
            unsigned char *argbData = new unsigned char[bitmap.width * bitmap.rows * 4];
            for (int y = 0; y < bitmap.rows; y++)
            {
                for (int x = 0; x < bitmap.width; x++)
                {
                    char value = bitmap.buffer[y * bitmap.width + x];
                    argbData[(y * bitmap.width + x) * 4] = 255;         // B
                    argbData[(y * bitmap.width + x) * 4 + 1] = 255;     // G
                    argbData[(y * bitmap.width + x) * 4 + 2] = 255;     // R
                    argbData[(y * bitmap.width + x) * 4 + 3] = value; // A
                }
            }

            SDL_UpdateTexture(texture, &charRect, argbData, bitmap.width * sizeof(Uint32));

            delete argbData;
            offsetX += glyph_slot->bitmap.width;
        }

        // 创建SDL纹理并将位图数据复制到纹理中

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        return {texture, width, height};
        // return {texture, width * 0.48, height * 0.48};
    }
}