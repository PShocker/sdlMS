#include "ChatBalloon.h"
#include "Core/FreeType.h"
#include "Resources/Wz.h"
#include "wz/Property.hpp"

void ChatBalloon::push(ChatBalloon *chatballoon, std::u16string str, SDL_Color color, std::u16string type)
{
    FreeType::size(15);
    auto str_texture = FreeType::load(str, color, 140);
    auto back_texture = load_back_texture(str_texture);
    chatballoon->chatballoons.push_back({str_texture, back_texture});
}

SDL_Texture *ChatBalloon::load_back_texture(SDL_Texture *str_texture, std::u16string type)
{
    auto node = Wz::UI->get_root()->find_from_path(u"ChatBalloon.img/" + type);

    auto c = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"c"));
    auto e = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e"));
    auto ne = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"ne"));
    auto n = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"n"));
    auto nw = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"nw"));
    auto w = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w"));
    auto sw = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"sw"));
    auto s = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"s"));
    auto se = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"se"));
    auto arrow = dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"arrow"));

    auto width = str_texture->w + nw->get().width + ne->get().width;
    auto height = str_texture->h + nw->get().height + sw->get().height;

    auto line = ceil((float)height / (float)c->get().height);
    height = line * c->get().height;

    auto texture = SDL_CreateTexture(Window::renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_TARGET, width, height + 8);
    SDL_SetRenderTarget(Window::renderer, texture);
    SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 0);
    SDL_RenderClear(Window::renderer);
    SDL_SetRenderTarget(Window::renderer, NULL);
    SDL_Rect rect;

    for (int x = 0; x <= width; x += c->get().width)
    {
        for (int y = 0; y <= height - c->get().height; y += c->get().height)
        {
            rect = {x, y, c->get().width, c->get().height};
            SDL_UpdateTexture(texture, &rect, c->get_raw_data().data(), c->get().width * sizeof(Uint16));
        }
    }

    for (int i = nw->get().height; i <= height - w->get().height; i += w->get().height)
    {
        rect = {0, i, w->get().width, w->get().height};
        SDL_UpdateTexture(texture, &rect, w->get_raw_data().data(), w->get().width * sizeof(Uint16));

        rect = {width - e->get().width, i, e->get().width, e->get().height};
        SDL_UpdateTexture(texture, &rect, e->get_raw_data().data(), e->get().width * sizeof(Uint16));
    }

    for (int i = 0; i <= width; i += n->get().width)
    {
        rect = {i, 0, n->get().width, n->get().height};
        SDL_UpdateTexture(texture, &rect, n->get_raw_data().data(), n->get().width * sizeof(Uint16));

        rect = {i, height - s->get().height, s->get().width, s->get().height};
        SDL_UpdateTexture(texture, &rect, s->get_raw_data().data(), s->get().width * sizeof(Uint16));
    }

    rect = {0, nw->get().height, w->get().width, w->get().height};
    SDL_UpdateTexture(texture, &rect, w->get_raw_data().data(), w->get().width * sizeof(Uint16));

    rect = {0, height - sw->get().height - w->get().height, w->get().width, w->get().height};
    SDL_UpdateTexture(texture, &rect, w->get_raw_data().data(), w->get().width * sizeof(Uint16));

    rect = {width - e->get().width, nw->get().height, e->get().width, e->get().height};
    SDL_UpdateTexture(texture, &rect, e->get_raw_data().data(), e->get().width * sizeof(Uint16));

    rect = {width - e->get().width, height - sw->get().height - e->get().height, e->get().width, e->get().height};
    SDL_UpdateTexture(texture, &rect, e->get_raw_data().data(), e->get().width * sizeof(Uint16));

    rect = {0, 0, nw->get().width, nw->get().height};
    SDL_UpdateTexture(texture, &rect, nw->get_raw_data().data(), nw->get().width * sizeof(Uint16));

    rect = {0, height - sw->get().height, sw->get().width, sw->get().height};
    SDL_UpdateTexture(texture, &rect, sw->get_raw_data().data(), sw->get().width * sizeof(Uint16));

    rect = {width - ne->get().width, 0, ne->get().width, ne->get().height};
    SDL_UpdateTexture(texture, &rect, ne->get_raw_data().data(), ne->get().width * sizeof(Uint16));

    rect = {width - se->get().width, height - se->get().height, se->get().width, se->get().height};
    SDL_UpdateTexture(texture, &rect, se->get_raw_data().data(), se->get().width * sizeof(Uint16));

    rect = {width / 2 - arrow->get().width / 2, height - arrow->get().height + 8, arrow->get().width - 1, arrow->get().height};
    SDL_UpdateTexture(texture, &rect, arrow->get_raw_data().data() + 2, arrow->get().width * sizeof(Uint16));

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    return texture;
}
