#include "sdlms/obj.hpp"

Obj::Obj(std::vector<Sprite> sprite,
         std::vector<int> delay,
         int layer,
         int z, int flip, std::u16string url,
         int frameSize,
         std::vector<std::tuple<int, int>> a) : AnimatedSprite(sprite, delay,
                                                               frameSize,
                                                               a, flip),
                                                _layer(layer), _z(z),
                                                _url(url)
{
}
