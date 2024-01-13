#include "sdlms/obj.hpp"

Obj::Obj(std::vector<SDL_Texture *> texture, std::vector<SDL_Rect *> rect,
         std::vector<int> delay,
         std::vector<int> format, int layer,
         int z, int flip, std::u16string url, int frameSize) : AnimatedSprite(texture, rect, delay,
                                                                              format, frameSize, flip),
                                                               _layer(layer), _z(z),
                                                               _url(url)
{
}
