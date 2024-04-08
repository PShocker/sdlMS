#include "sdlms/obj.hpp"

Obj::Obj(int id,
         AnimatedSprite animatedsprite,
         int layer,
         int z, std::u16string url) : _id(id),
                                      AnimatedSprite(animatedsprite),
                                      _layer(layer), _z(z),
                                      _url(url)
{
}
