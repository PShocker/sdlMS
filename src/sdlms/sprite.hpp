#ifndef HEADER_SDLMS_SPRITE
#define HEADER_SDLMS_SPRITE

#include <SDL2/SDL.h>
#include <vector>
#include <map>
#include <string>
#include <any>
#include "template/point.hpp"
#include "wz/Property.hpp"

class Sprite
{
public:
    Sprite(std::u16string url, const std::vector<uint8_t> &raw_data, SDL_FRect rect,
           unsigned int format, int flip = SDL_FLIP_NONE,
           int delay = 0, int a0 = 0, int a1 = 0);
    Sprite(SDL_Texture *texture, SDL_FRect rect, int flip = SDL_FLIP_NONE);
    Sprite(){};

    void draw();
    void draw(Point<float> position);
    void draw_static();
    void draw_static(Point<float> position);

    SDL_FRect rect();

public:
    // 从wz中加载sprite
    static Sprite load_sprite(wz::Node *node, int x = 0, int y = 0, int flip = SDL_FLIP_NONE);
    static Sprite load_sprite(wz::Node *node, Point<int32_t> p, int flip = SDL_FLIP_NONE);

public:
    SDL_Texture *_texture;
    SDL_FRect _rect;
    std::vector<uint8_t> _raw_data;
    unsigned int _format;
    int _flip;
    int _delay;
    int _a0;
    int _a1;

    std::u16string _url;

    static inline std::map<std::u16string, SDL_Texture *> texture_map;

    // std::map<std::u16string, std::any> _others;
};
#endif