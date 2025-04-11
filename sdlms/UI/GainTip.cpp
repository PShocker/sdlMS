#include "GainTip.h"
#include "Cursor.h"
#include "Resources/Wz.h"
#include "Core/Core.h"

static inline SDL_Texture *prefix_cache[7];
static inline SDL_Texture *bracket_cache[2];
static inline SDL_Texture *number_cache[10];
static inline SDL_Texture *plus_cache;

void GainTip::init()
{
    number_cache[0] = FreeType::load(u"0", SDL_Color{255, 255, 255, 255}, 0, 12);
    number_cache[1] = FreeType::load(u"1", SDL_Color{255, 255, 255, 255}, 0, 12);
    number_cache[2] = FreeType::load(u"2", SDL_Color{255, 255, 255, 255}, 0, 12);
    number_cache[3] = FreeType::load(u"3", SDL_Color{255, 255, 255, 255}, 0, 12);
    number_cache[4] = FreeType::load(u"4", SDL_Color{255, 255, 255, 255}, 0, 12);
    number_cache[5] = FreeType::load(u"5", SDL_Color{255, 255, 255, 255}, 0, 12);
    number_cache[6] = FreeType::load(u"6", SDL_Color{255, 255, 255, 255}, 0, 12);
    number_cache[7] = FreeType::load(u"7", SDL_Color{255, 255, 255, 255}, 0, 12);
    number_cache[8] = FreeType::load(u"8", SDL_Color{255, 255, 255, 255}, 0, 12);
    number_cache[9] = FreeType::load(u"9", SDL_Color{255, 255, 255, 255}, 0, 12);

    bracket_cache[0] = FreeType::load(u"(", SDL_Color{255, 255, 255, 255}, 0, 12);
    bracket_cache[1] = FreeType::load(u")", SDL_Color{255, 255, 255, 255}, 0, 12);

    prefix_cache[0] = FreeType::load(u"得到经验值", SDL_Color{255, 255, 255, 255}, 0, 12);
    prefix_cache[1] = FreeType::load(u"得到装备栏物品", SDL_Color{255, 255, 255, 255}, 0, 12);
    prefix_cache[2] = FreeType::load(u"得到消耗栏物品", SDL_Color{255, 255, 255, 255}, 0, 12);
    prefix_cache[3] = FreeType::load(u"得到设置栏物品", SDL_Color{255, 255, 255, 255}, 0, 12);
    prefix_cache[4] = FreeType::load(u"得到其他栏物品", SDL_Color{255, 255, 255, 255}, 0, 12);
    prefix_cache[5] = FreeType::load(u"得到特殊栏物品", SDL_Color{255, 255, 255, 255}, 0, 12);
    prefix_cache[6] = FreeType::load(u"得到金币", SDL_Color{255, 255, 255, 255}, 0, 12);

    plus_cache = FreeType::load(u"+", SDL_Color{255, 255, 255, 255}, 0, 12);
}

void GainTip::run()
{
    for (auto it = GainTip::gaintips.begin(); it != GainTip::gaintips.end();)
    {
        if (it->destory <= Window::dt_now)
        {
            SDL_DestroyTexture(it->texture);
            it = GainTip::gaintips.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void GainTip::push(const std::u16string &index, int num)
{
    if (index == u"e" || index == u"09000000")
    {
        // 经验值
        auto prefix = index == u"e" ? prefix_cache[0] : prefix_cache[6];
        auto num_texture = load_num_texture(num);
        auto texture = Texture::createBlankTexture(SDL_PIXELFORMAT_ARGB8888,
                                                   prefix->w + bracket_cache[0]->w + num_texture->w + bracket_cache[1]->w,
                                                   prefix->h);
        SDL_SetRenderTarget(Window::renderer, texture);
        int i = 0;
        SDL_FRect pos_rect{(float)i, (float)0, (float)prefix->w, (float)prefix->h};
        SDL_RenderTexture(Window::renderer, prefix, nullptr, &pos_rect);
        i += prefix->w;
        pos_rect = {(float)i, (float)0, (float)bracket_cache[0]->w, (float)bracket_cache[0]->h};
        SDL_RenderTexture(Window::renderer, bracket_cache[0], nullptr, &pos_rect);
        i += bracket_cache[0]->w;
        pos_rect = {(float)i, (float)0, (float)plus_cache->w, (float)plus_cache->h};
        SDL_RenderTexture(Window::renderer, plus_cache, nullptr, &pos_rect);
        i += plus_cache->w;
        pos_rect = {(float)i, (float)0, (float)num_texture->w, (float)num_texture->h};
        SDL_RenderTexture(Window::renderer, num_texture, nullptr, &pos_rect);
        i += num_texture->w;
        pos_rect = {(float)i, (float)0, (float)bracket_cache[1]->w, (float)bracket_cache[1]->h};
        SDL_RenderTexture(Window::renderer, bracket_cache[1], nullptr, &pos_rect);
        SDL_SetRenderTarget(Window::renderer, nullptr);

        SDL_DestroyTexture(num_texture);
        gaintips.push_back({texture, Window::dt_now + 8000});
    }
    else
    {
    }
}

SDL_Texture *GainTip::load_num_texture(int num)
{
    int length = 0;
    int height = 0;
    // 分解各位数字
    while (num > 0)
    {
        int digit = num % 10;
        length += number_cache[digit]->w;
        height = std::max(height, number_cache[digit]->h);
        num /= 10;
    }
    auto texture = Texture::createBlankTexture(SDL_PIXELFORMAT_ARGB8888, length, height);
    SDL_SetRenderTarget(Window::renderer, texture);
    while (num > 0)
    {
        int digit = num % 10;
        length -= number_cache[digit]->w;
        SDL_FRect pos_rect{(float)length, (float)0, (float)number_cache[digit]->w, (float)number_cache[digit]->h};
        SDL_RenderTexture(Window::renderer, number_cache[digit], nullptr, &pos_rect);
        num /= 10;
    }
    SDL_SetRenderTarget(Window::renderer, nullptr);
    return texture;
}
