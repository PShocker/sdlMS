#ifndef HEADER_SDLMS_TTF
#define HEADER_SDLMS_TTF

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "sdlms/graphics.hpp"
#include "util/currenton.hpp"

class TTF : public Currenton<TTF>
{
public:
    TTF();
    SDL_Texture *load_font(const std::string &s);

public:
    const std::string filename_prefix = "C:/Users/Shocker/Desktop/sdlMS/data/";
    TTF_Font *_font;

private:
    SDL_Renderer *_renderer;
};
#endif