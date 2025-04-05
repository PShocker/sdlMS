#include "ChatBalloon.h"
#include "Core/FreeType.h"
#include "Core/Texture.h"
#include "Resources/Wz.h"
#include "wz/Property.hpp"

void ChatBalloon::push(ChatBalloon *chatballoon, std::u16string str, SDL_Color color, std::u16string type)
{
    auto str_texture = FreeType::load(str, color, 140, 15);
    auto back_texture = load_back_texture(str_texture, u"0");
    chatballoon->chatballoons.push_back({str_texture, back_texture});
}

SDL_Texture *ChatBalloon::load_back_texture(SDL_Texture *str_texture, std::u16string type)
{
    auto node = Wz::UI->get_root()->find_from_path(u"ChatBalloon.img/" + type);

    auto c = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"c")));
    auto e = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e")));
    auto ne = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"ne")));
    auto n = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"n")));
    auto nw = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"nw")));
    auto w = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w")));
    auto sw = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"sw")));
    auto s = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"s")));
    auto se = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"se")));
    auto arrow = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"arrow")));

    auto width = str_texture->w + nw->w + ne->w;
    auto height = str_texture->h + nw->h + sw->h;
    // arrow高度
    height += arrow->h / 2;

    auto texture = Texture::createBlankTexture(SDL_PIXELFORMAT_ARGB4444, width, height);
    SDL_SetRenderTarget(Window::renderer, texture);
    SDL_SetRenderDrawBlendMode(Window::renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 0);
    SDL_FRect pos_rect;
    // 先以c填充整个texture,然后在填充四周的边框,最后是四个边角
    for (int x = 0; x <= width; x += c->w)
    {
        for (int y = 0; y <= height; y += c->h)
        {
            pos_rect = {(float)x, (float)y, (float)c->w, (float)c->h};
            SDL_RenderTexture(Window::renderer, c, nullptr, &pos_rect);
        }
    }
    // 竖着的两边
    for (int i = 0; i <= height; i += w->h)
    {
        pos_rect = {(float)0, (float)i, (float)w->w, (float)w->h};
        SDL_RenderFillRect(Window::renderer, &pos_rect);
        SDL_RenderTexture(Window::renderer, w, nullptr, &pos_rect);

        pos_rect = {(float)width - e->w, (float)i, (float)e->w, (float)e->h};
        SDL_RenderFillRect(Window::renderer, &pos_rect);
        SDL_RenderTexture(Window::renderer, e, nullptr, &pos_rect);
    }
    // 横着的两边
    for (int i = 0; i <= width; i += n->w)
    {
        pos_rect = {(float)i, 0, (float)n->w, (float)n->h};
        SDL_RenderFillRect(Window::renderer, &pos_rect);
        SDL_RenderTexture(Window::renderer, n, nullptr, &pos_rect);

        pos_rect = {(float)i, (float)height - arrow->h / 2 - s->h, (float)s->w, (float)s->h};
        SDL_RenderFillRect(Window::renderer, &pos_rect);
        SDL_RenderTexture(Window::renderer, s, nullptr, &pos_rect);
    }
    // 左上
    pos_rect = {0, 0, (float)nw->w, (float)nw->h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_RenderTexture(Window::renderer, nw, nullptr, &pos_rect);
    // 左下
    pos_rect = {0, (float)height - arrow->h / 2 - sw->h, (float)sw->w, (float)sw->h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_RenderTexture(Window::renderer, sw, nullptr, &pos_rect);
    // 右上
    pos_rect = {(float)width - ne->w, 0, (float)ne->w, (float)ne->h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_RenderTexture(Window::renderer, ne, nullptr, &pos_rect);
    // 右下
    pos_rect = {(float)width - se->w, (float)height - arrow->h / 2 - se->h, (float)se->w, (float)se->h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_RenderTexture(Window::renderer, se, nullptr, &pos_rect);

    pos_rect = {(float)0, (float)height - arrow->h / 2, (float)width, (float)arrow->h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);

    // 渲染arrow
    pos_rect = {(float)width / 2 - arrow->w / 2, (float)height - arrow->h, (float)arrow->w, (float)arrow->h};
    SDL_FRect src_rect = {(float)2, (float)0, (float)arrow->w - 2, (float)arrow->h};
    SDL_RenderTexture(Window::renderer, arrow, &src_rect, &pos_rect);
    SDL_SetRenderTarget(Window::renderer, nullptr);

    return texture;
}
