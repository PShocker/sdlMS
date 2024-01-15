#include "sdlms/obj.hpp"

Obj::Obj(std::vector<SDL_Texture *> texture, std::vector<SDL_Rect *> rect,
         std::vector<int> delay,
         std::vector<int> format, int layer,
         int z, int flip, std::u16string url,
         int frameSize,
         std::vector<std::tuple<int, int>> a) : AnimatedSprite(texture, rect, delay,
                                                               format, frameSize,
                                                               a, flip),
                                                _layer(layer), _z(z),
                                                _url(url)
{
}
