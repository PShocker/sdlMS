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
    Sprite(std::u16string url, const std::vector<uint8_t> &raw_data, SDL_FRect rect, unsigned int format, int flip = SDL_FLIP_NONE, int delay = 0);
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
    int _delay;

    std::u16string _url;

    static inline std::map<std::u16string, SDL_Texture *> texture_map;

    // std::map<std::u16string, std::any> _others;
};
#endif