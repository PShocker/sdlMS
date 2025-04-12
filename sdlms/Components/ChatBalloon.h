#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include "Core/Window.h"
#include "entt/entt.hpp"

struct ChatBalloon
{
    struct Wrap
    {
        SDL_Texture *str_texture;
        SDL_Texture *back_texture;
        bool show = true;
        unsigned int delay = Window::dt_now;
    };

    static void push(ChatBalloon *chatballoon, std::u16string str, SDL_Color color, std::u16string type = u"0");

    static SDL_Texture *load_back_texture(SDL_Texture *str_texture, std::u16string type = u"0");

    std::vector<Wrap> chatballoons;
};