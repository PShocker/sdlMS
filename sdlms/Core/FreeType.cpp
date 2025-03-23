#include "FreeType.h"
#include <vector>
#include <math.h>

void FreeType::init(const std::string &filename_prefix)
{
    FT_Init_FreeType(library);
    FT_New_Face(*library, (filename_prefix + "simsun.ttc").c_str(), 0, face);
    FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
    FT_Set_Pixel_Sizes(*face, 0, 18);
    return;
}

void FreeType::renderGlyphToTexture(SDL_Texture *texture, char16_t c, SDL_Color color, int offsetX, int offsetY, int height, FT_GlyphSlot &glyph_slot)
{
    auto index = FT_Get_Char_Index(*face, c);
    FT_Load_Glyph(*face, index, FT_LOAD_DEFAULT);
    FT_Render_Glyph(glyph_slot, FT_RENDER_MODE_MONO);
    auto bitmap = glyph_slot->bitmap;

    SDL_Rect char_rect = {offsetX, offsetY + (int)(height - glyph_slot->bitmap.rows) / 2, (int)glyph_slot->bitmap.width, (int)glyph_slot->bitmap.rows};
    if (c == u'\u3002' || c == u'\uFF0C' || c == u'\u002C')
    {
        char_rect.y = offsetY + height - glyph_slot->bitmap.rows * 2;
    }

    std::vector<unsigned char> argb_data(bitmap.width * bitmap.rows * 4, 0);
    for (int y = 0; y < bitmap.rows; y++)
    {
        for (int x = 0; x < bitmap.width; x++)
        {
            unsigned char value = bitmap.buffer[y * bitmap.pitch + (x >> 3)] & (0x80 >> (x & 7));
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
    SDL_UpdateTexture(texture, &char_rect, argb_data.data(), bitmap.width * sizeof(Uint32));
}

SDL_Texture *FreeType::load(const std::u16string &s, SDL_Color color, int w)
{
    SDL_Renderer *renderer = Window::renderer;
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
            width += (glyph_slot->advance.x >> 6);
        }
        height = std::max((int)glyph_slot->bitmap.rows, height);
    }
    if (w <= 0)
    {
        SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
        SDL_SetRenderTarget(renderer, texture);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, NULL);
        int offsetX = 0;
        for (auto &c : s)
        {
            renderGlyphToTexture(texture, c, color, offsetX, 0, height, glyph_slot);
            offsetX += (glyph_slot->advance.x >> 6);
        }
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        return texture;
    }
    else
    {
        // 计算高度
        auto line = 1;
        int offsetX = 0;
        for (int i = 0; i < s.length(); i++)
        {
            // 处理换行符
            if (s[i] == '\n')
            {
                offsetX = 0;
                line += 1;
                continue;
            }
            // 获取字符索引并加载字形
            auto index = FT_Get_Char_Index(*face, s[i]);
            FT_Load_Glyph(*face, index, FT_LOAD_DEFAULT);
            FT_Render_Glyph(glyph_slot, FT_RENDER_MODE_NORMAL);
            // 获取字符宽度
            int charWidth = glyph_slot->advance.x >> 6;
            // 换行逻辑：如果当前字符超出宽度，提前换行
            if (offsetX + charWidth > width)
            {
                offsetX = 0;
                line += 1;
            }
            // 累加字符宽度
            offsetX += charWidth;
        }
        offsetX = 0;
        int offsetY = 0;
        height += 6;
        SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height * line);
        SDL_SetRenderTarget(renderer, texture);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, NULL);
        for (int i = 0; i < s.length(); i++)
        {
            // 首先判断当前要渲染的字是否超过最大宽度
            auto index = FT_Get_Char_Index(*face, s[i]);
            FT_Load_Glyph(*face, index, FT_LOAD_DEFAULT);
            FT_Render_Glyph(glyph_slot, FT_RENDER_MODE_MONO);
            if (offsetX + (glyph_slot->advance.x >> 6) > width)
            {
                offsetY += height;
                offsetX = 0;
            }
            renderGlyphToTexture(texture, s[i], color, offsetX, offsetY, height, glyph_slot);
            offsetX += (glyph_slot->advance.x >> 6);
        }
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        return texture;
    }
}

void FreeType::size(int size)
{
    FT_Set_Pixel_Sizes(*face, 0, size);
}