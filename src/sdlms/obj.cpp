#include "sdlms/obj.hpp"

Obj::Obj(std::vector<Sprite> sprite,
         std::vector<int> delay,
         int layer,
         int z, std::u16string url,
         int frameSize,
         std::vector<std::tuple<int, int>> a) : AnimatedSprite(sprite, delay,
                                                               frameSize,
                                                               a),
                                                _layer(layer), _z(z),
                                                _url(url)
{
}
