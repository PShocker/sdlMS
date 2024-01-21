#include "sdlms/minimap.hpp"

MiniMap::MiniMap()
{
}

void MiniMap::draw(){
    // 设置绘制颜色
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 红色

    // 定义圆角矩形的参数
    int x = 100;        // 矩形左上角的x坐标
    int y = 100;        // 矩形左上角的y坐标
    int w = 400;        // 矩形的宽度
    int h = 300;        // 矩形的高度
    int radius = 50;    // 圆角的半径

    // 绘制圆角矩形的四个角
    SDL_RenderDrawLine(renderer, x + radius, y, x + w - radius, y);                                 // 上边
    SDL_RenderDrawLine(renderer, x + radius, y + h, x + w - radius, y + h);                         // 下边
    SDL_RenderDrawLine(renderer, x, y + radius, x, y + h - radius);                                 // 左边
    SDL_RenderDrawLine(renderer, x + w, y + radius, x + w, y + h - radius);                         // 右边

    // 绘制圆角
    SDL_RenderDrawArc(renderer, x + radius, y + radius, radius, 180, 270);                           // 左上角
    SDL_RenderDrawArc(renderer, x + w - radius, y + radius, radius, 270, 360);                       // 右上角
    SDL_RenderDrawArc(renderer, x + w - radius, y + h - radius, radius, 0, 90);                      // 右下角
    SDL_RenderDrawArc(renderer, x + radius, y + h - radius, radius, 90, 180);                        // 左下角

    // 填充圆角矩形
    SDL_Rect fillRect = { x + radius, y, w - radius * 2, h };
    SDL_RenderFillRect(renderer, &fillRect);
}