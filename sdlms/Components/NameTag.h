#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <string>

struct NameTag
{
    struct Wrap
    {
        SDL_Texture *str_texture;
        SDL_Texture *back_texture;
    };

    int offset_x = 0;
    int offset_y = 0;

    static void push(NameTag *nametag, std::u16string str, SDL_Color color, std::u16string type = u"");

    std::vector<Wrap> nametags;
};