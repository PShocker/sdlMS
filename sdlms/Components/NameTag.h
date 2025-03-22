#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <string>

struct NameTag
{
    struct Info
    {
        SDL_Texture *str_texture;
        SDL_Texture *back_texture;
    };

    static void push(NameTag *nametag, std::u16string str, SDL_Color color, std::u16string type = u"");

    std::vector<Info> nametags;
};