#include "FreeType.h"

void FreeType::init(const std::string &filename_prefix)
{
    library = new FT_Library{};
    FT_Init_FreeType(library);
    // 加载字体文件
    face = new FT_Face{};
#ifdef __WIN32__
    FT_New_Face(*library, "C:/Windows/Fonts/simsun.ttc", 0, face);
#else
    FT_New_Face(*library, (filename_prefix + "simsun.ttc").c_str(), 0, face);
#endif
    FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
    // 设置字体大小18
    FT_Set_Pixel_Sizes(*face, 0, 18);
    return;
}

void FreeType::size(int size)
{
    FT_Set_Pixel_Sizes(*face, 0, size);
}

Sprite *FreeType::str(const std::u16string &s, SDL_Color color, int w)
{
    SDL_Renderer *renderer = Window::get_renderer();

    FT_GlyphSlot glyph_slot = (*face)->glyph;
    // 计算每个字符的位置和大小
    int width = 0;
    int height = 0;
    for (int i = 0; i < s.length(); i++)
    {
        auto c = s[i];
        auto index = FT_Get_Char_Index(*face, c);
        FT_Load_Glyph(*face, index, FT_LOAD_DEFAULT);
        if (w <= 0 || (w > 0 && i < w))
        {
            width += glyph_slot->advance.x >> 6;
        }
        height = std::max((int)glyph_slot->bitmap.rows, height);
    }
    if (w <= 0)
    {
        SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
        int offsetX = 0;
        for (auto &c : s)
        {
            auto index = FT_Get_Char_Index(*face, c);
            FT_Load_Glyph(*face, index, FT_LOAD_DEFAULT);
            FT_Render_Glyph(glyph_slot, FT_RENDER_MODE_MONO);
            auto bitmap = glyph_slot->bitmap;
            // 默认顶部对齐
            SDL_Rect char_rect = {offsetX, (int)(height - glyph_slot->bitmap.rows) / 2, (int)glyph_slot->bitmap.width, (int)glyph_slot->bitmap.rows};
            // 特殊符号居中
            if (c == u'<' || c == u'>')
            {
                char_rect.y = (height - glyph_slot->bitmap.rows) / 2;
            }
            // 转换为ARGB8888格式
            unsigned char *argb_data = new unsigned char[bitmap.width * bitmap.rows * 4];
            for (int y = 0; y < bitmap.rows; y++)
            {
                for (int x = 0; x < bitmap.width; x++)
                {
                    unsigned char value = bitmap.buffer[y * bitmap.pitch + (x >> 3)] & (0x80 >> (x & 7));
                    [[likely]]
                    if (value == 0)
                    {
                        argb_data[(y * bitmap.width + x) * 4] = 0;     // B
                        argb_data[(y * bitmap.width + x) * 4 + 1] = 0; // G
                        argb_data[(y * bitmap.width + x) * 4 + 2] = 0; // R
                        argb_data[(y * bitmap.width + x) * 4 + 3] = 0; // A
                    }
                    else
                    {
                        argb_data[(y * bitmap.width + x) * 4] = color.b;     // B
                        argb_data[(y * bitmap.width + x) * 4 + 1] = color.g; // G
                        argb_data[(y * bitmap.width + x) * 4 + 2] = color.r; // R
                        argb_data[(y * bitmap.width + x) * 4 + 3] = color.a; // A
                    }
                }
            }
            SDL_UpdateTexture(texture, &char_rect, argb_data, bitmap.width * sizeof(Uint32));
            offsetX += glyph_slot->advance.x >> 6;
            delete[] argb_data;
        }
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        return new Sprite(texture, width, height);
    }
    else
    {
        auto length = s.length();
        auto line = std::ceil((float)length / (float)w);
        int offsetX = 0;
        int offsetY = 0;
        height += 6;
        width += 12;
        SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height * line);
        for (int i = 0; i < s.length(); i++)
        {
            // 获取第l行数
            auto l = i / w + 1;
            if (i % w == 0)
            {
                offsetX = 0;
            }
            offsetY = (l - 1) * height;
            auto &c = s[i];
            auto index = FT_Get_Char_Index(*face, c);
            FT_Load_Glyph(*face, index, FT_LOAD_DEFAULT);
            FT_Render_Glyph(glyph_slot, FT_RENDER_MODE_MONO);
            auto bitmap = glyph_slot->bitmap;
            // 默认顶部对齐
            SDL_Rect char_rect = {offsetX, (int)(height - glyph_slot->bitmap.rows) / 2 + offsetY, (int)glyph_slot->bitmap.width, (int)glyph_slot->bitmap.rows};
            // 特殊符号居中
            if (c == u'<' || c == u'>')
            {
                char_rect.y = (height - glyph_slot->bitmap.rows) / 2;
            }
            // 转换为ARGB8888格式
            unsigned char *argb_data = new unsigned char[bitmap.width * bitmap.rows * 4];
            for (int y = 0; y < bitmap.rows; y++)
            {
                for (int x = 0; x < bitmap.width; x++)
                {
                    unsigned char value = bitmap.buffer[y * bitmap.pitch + (x >> 3)] & (0x80 >> (x & 7));
                    [[likely]]
                    if (value == 0)
                    {
                        argb_data[(y * bitmap.width + x) * 4] = 0;     // B
                        argb_data[(y * bitmap.width + x) * 4 + 1] = 0; // G
                        argb_data[(y * bitmap.width + x) * 4 + 2] = 0; // R
                        argb_data[(y * bitmap.width + x) * 4 + 3] = 0; // A
                    }
                    else
                    {
                        argb_data[(y * bitmap.width + x) * 4] = color.b;     // B
                        argb_data[(y * bitmap.width + x) * 4 + 1] = color.g; // G
                        argb_data[(y * bitmap.width + x) * 4 + 2] = color.r; // R
                        argb_data[(y * bitmap.width + x) * 4 + 3] = color.a; // A
                    }
                }
            }
            SDL_UpdateTexture(texture, &char_rect, argb_data, bitmap.width * sizeof(Uint32));
            offsetX += glyph_slot->advance.x >> 6;
            delete[] argb_data;
        }
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        return new Sprite(texture, width, height * line);
    }
}
