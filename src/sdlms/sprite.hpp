#ifndef HEADER_SDLMS_SPRITE
#define HEADER_SDLMS_SPRITE

#include <SDL2/SDL.h>
#include <vector>
#include <map>
#include <string>
#include <any>




class Sprite
{
public:
    Sprite(const std::vector<uint8_t> &raw_data, SDL_FRect rect, unsigned int format, int flip = SDL_FLIP_NONE);
    Sprite(SDL_Texture *texture, SDL_FRect rect, int flip);

    void draw();
    void _draw();

    SDL_FRect rect();

public:
    SDL_Texture *_texture;
    SDL_FRect _rect;
    std::vector<uint8_t> _raw_data;
    unsigned int _format;
    int _flip;
    // std::map<std::u16string, std::any> _others;
};
#endif

/*
// 原始图像的宽度和高度
const int original_width = 320;
const int original_height = 240;

// 缩略图的宽度和高度
const int thumbnail_width = 160;
const int thumbnail_height = 120;

// 原始RGB565像素数据
std::vector<uint16_t> original_pixels(original_width * original_height);

// 缩略图RGB565像素数据
std::vector<uint16_t> thumbnail_pixels(thumbnail_width * thumbnail_height);

// 将缩略图的RGB565像素数据还原为原始图像的RGB565像素数据
void restoreOriginalPixels() {
    for (int y = 0; y < thumbnail_height; ++y) {
        for (int x = 0; x < thumbnail_width; ++x) {
            // 计算原始图像中对应的像素位置
            int original_x = x * original_width / thumbnail_width;
            int original_y = y * original_height / thumbnail_height;

            // 获取缩略图中的RGB565像素值
            uint16_t thumbnail_pixel = thumbnail_pixels[y * thumbnail_width + x];

            // 将RGB565像素值放置回原始图像对应位置
            original_pixels[original_y * original_width + original_x] = thumbnail_pixel;
        }
    }
}*/