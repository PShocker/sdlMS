#include "sdlms/obj.hpp"

Obj::Obj(std::vector<SDL_Texture *> texture, std::vector<SDL_Rect *> rect, std::vector<int> delay,
         std::vector<int> format, int layer, int z, std::u16string url) : _texture(texture), _rect(rect),
                                                                          _delay(delay), _format(format),
                                                                          _layer(layer), _z(z), _frameIndex(0),
                                                                          _url(url)
{
}
