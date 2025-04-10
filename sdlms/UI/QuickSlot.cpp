#include "QuickSlot.h"
#include "Resources/Wz.h"
#include "Systems/UI.h"

void QuickSlot::init()
{
    auto ui_node = Wz::UI->get_root();
    backgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/quickSlot")));
    // 计算坐标
    if (backgrnd2)
    {
        SDL_DestroyTexture(backgrnd2);
    }
    if (Camera::w - QuickSlot::backgrnd->w <= 824)
    {
        QuickSlot::x = Camera::w - QuickSlot::backgrnd->w;
        QuickSlot::y = Camera::h - QuickSlot::backgrnd->h - 65;
        QuickSlot::w = QuickSlot::backgrnd->w;
        QuickSlot::h = QuickSlot::backgrnd->h;
        backgrnd2 = Texture::createBlankTexture(SDL_PIXELFORMAT_ARGB4444, QuickSlot::w, QuickSlot::h);
        SDL_SetRenderTarget(Window::renderer, backgrnd2);
        SDL_FRect pos_rect{(float)0, (float)0, (float)QuickSlot::w, (float)QuickSlot::h};
        SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);
        SDL_SetRenderTarget(Window::renderer, nullptr);
    }
    else
    {
        // 中间的35是需要平铺的键位长度,左右6像素是左右边框
        auto tile_cnt_x = ((Camera::w - 824) - 12) / 35;
        QuickSlot::x = Camera::w - tile_cnt_x * 35 - 12;
        QuickSlot::y = Camera::h - QuickSlot::backgrnd->h;
        QuickSlot::w = Camera::w - QuickSlot::x;
        QuickSlot::h = QuickSlot::backgrnd->h;

        backgrnd2 = Texture::createBlankTexture(SDL_PIXELFORMAT_ARGB4444, QuickSlot::w, QuickSlot::h);
        SDL_SetRenderTarget(Window::renderer, backgrnd2);
        SDL_FRect src_rect{(float)0, (float)0, (float)6, (float)QuickSlot::h};
        SDL_FRect pos_rect{(float)0, (float)0, (float)6, (float)QuickSlot::h};
        SDL_RenderTexture(Window::renderer, backgrnd, &src_rect, &pos_rect);
        for (int i = 0; i < tile_cnt_x; i++)
        {
            src_rect = {(float)110, (float)0, (float)35, (float)QuickSlot::h};
            pos_rect = {(float)6 + i * 35, (float)0, (float)35, (float)QuickSlot::h};
            SDL_RenderTexture(Window::renderer, backgrnd, &src_rect, &pos_rect);
        }
        src_rect = {(float)145, (float)0, (float)6, (float)QuickSlot::h};
        pos_rect = {(float)QuickSlot::w - 6, (float)0, (float)6, (float)QuickSlot::h};
        SDL_RenderTexture(Window::renderer, backgrnd, &src_rect, &pos_rect);
        SDL_SetRenderTarget(Window::renderer, nullptr);
    }
}
