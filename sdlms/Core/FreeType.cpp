#include "FreeType.h"
#include "Texture.h"
#include <vector>
#include <algorithm>

void FreeType::init(const std::string &filename_prefix)
{
    FT_Init_FreeType(library);

    std::string font_path = filename_prefix + "simsun.ttc";
    FT_New_Face(*library, font_path.c_str(), 0, face);

    FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
    FT_Set_Pixel_Sizes(*face, 0, 18);
}

void FreeType::renderGlyphToTexture(SDL_Texture *texture, char16_t c,
                                    SDL_Color color, int offsetX, int offsetY)
{
    FT_GlyphSlot slot = (*face)->glyph;

    // 加载并渲染字形
    auto index = FT_Get_Char_Index(*face, c);
    if (index == 0)
        return; // 跳过无法显示的字形

    if (FT_Load_Glyph(*face, index, FT_LOAD_DEFAULT))
        return;
    if (FT_Render_Glyph(slot, FT_RENDER_MODE_MONO))
        return;

    FT_Bitmap &bitmap = slot->bitmap;
    int line_height = (*face)->size->metrics.height >> 6;
    int y_offset = offsetY + line_height - slot->bitmap.rows;

    // 准备像素数据
    std::vector<uint32_t> pixels(bitmap.width * bitmap.rows, 0);

    // 处理单色位图
    for (int y = 0; y < bitmap.rows; y++)
    {
        for (int x = 0; x < bitmap.width; x++)
        {
            uint8_t byte = bitmap.buffer[y * bitmap.pitch + (x / 8)];
            if (byte & (0x80 >> (x % 8)))
            {
                pixels[y * bitmap.width + x] =
                    (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
            }
        }
    }

    // 更新纹理
    SDL_Rect dstRect = {
        offsetX + slot->bitmap_left,
        y_offset,
        (int)bitmap.width,
        (int)bitmap.rows};

    SDL_UpdateTexture(texture, &dstRect, pixels.data(), bitmap.width * 4);
}

SDL_Texture *FreeType::load(const std::u16string &text, SDL_Color color, int max_width, int size)
{
    // 设置大小
    FT_Set_Pixel_Sizes(*face, 0, size);

    // 1. 计算文本尺寸
    int line_height = (*face)->size->metrics.height >> 6;
    int current_line_width = 0;
    int total_width = 0;
    int line_count = 1;

    // 第一遍：计算布局
    for (char16_t c : text)
    {
        if (c == '\n')
        {
            total_width = std::max(total_width, current_line_width);
            current_line_width = 0;
            line_count++;
            continue;
        }

        auto index = FT_Get_Char_Index(*face, c);
        if (index == 0)
            continue;

        if (FT_Load_Glyph(*face, index, FT_LOAD_DEFAULT))
            continue;

        int advance = ((*face)->glyph->advance.x >> 6);

        // 自动换行检查
        if (max_width > 0 && current_line_width + advance > max_width)
        {
            total_width = std::max(total_width, current_line_width);
            current_line_width = advance;
            line_count++;
        }
        else
        {
            current_line_width += advance;
        }
    }

    total_width = std::max(total_width, current_line_width);
    int total_height = line_height * line_count;

    // 2. 创建纹理
    SDL_Texture *texture = Texture::createBlankTexture(SDL_PIXELFORMAT_ARGB8888, total_width, total_height);

    // 3. 渲染文本
    int current_x = 0;
    int current_y = 0;

    for (char16_t c : text)
    {
        if (c == '\n')
        {
            current_x = 0;
            current_y += line_height;
            continue;
        }

        auto index = FT_Get_Char_Index(*face, c);
        if (index == 0)
            continue;

        if (FT_Load_Glyph(*face, index, FT_LOAD_DEFAULT))
            continue;

        int advance = ((*face)->glyph->advance.x >> 6);
        // 自动换行检查
        if (max_width > 0 && current_x + advance > max_width)
        {
            current_x = 0;
            current_y += line_height;
        }
        renderGlyphToTexture(texture, c, color, current_x, current_y);
        current_x += advance;
    }

    return texture;
}