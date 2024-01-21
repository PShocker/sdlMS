#include "sdlms/minimap.hpp"
#include "sdlms/graphics.hpp"
#include <math.h>

MiniMap::MiniMap()
{
}

void MiniMap::draw()
{

    auto renderer = Graphics::current()->getRenderer();

    // 设置绘制颜色
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 红色

    // 定义圆角矩形的参数
    int x = 0;       // 矩形左上角的x坐标
    int y = 0;       // 矩形左上角的y坐标
    int w = 180;     // 矩形的宽度
    int h = 250;     // 矩形的高度
    int radius = 20; // 圆角的半径

    // 绘制四条直线
    SDL_RenderDrawLine(renderer, x + radius, y, x + w - radius, y);         // 上边
    SDL_RenderDrawLine(renderer, x + radius, y + h, x + w - radius, y + h); // 下边
    SDL_RenderDrawLine(renderer, x, y + radius, x, y + h - radius);         // 左边
    SDL_RenderDrawLine(renderer, x + w, y + radius, x + w, y + h - radius); // 右边

    // 绘制圆角
    const double PI = M_PI;
    const double angleStep = PI / (2 * radius); // 每度的弧长为半径的2倍


    for (int i = 0; i < radius; i++)
    {
        double angle = i * angleStep;

        int x1 = x + radius - static_cast<int>(radius * cos(angle));
        int y1 = y + radius - static_cast<int>(radius * sin(angle));
        int x2 = x + radius - static_cast<int>(radius * sin(angle));
        int y2 = y + radius - static_cast<int>(radius * cos(angle));

        SDL_RenderDrawPoint(renderer, x1, y1);
        SDL_RenderDrawPoint(renderer, x + w - radius + static_cast<int>(radius * cos(angle)), y1);
        SDL_RenderDrawPoint(renderer, x1, y + h - radius + static_cast<int>(radius * sin(angle)));
        SDL_RenderDrawPoint(renderer, x + w - radius + static_cast<int>(radius * sin(angle)), y + h - radius + static_cast<int>(radius * cos(angle)));
    }
}