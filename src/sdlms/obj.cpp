#include "sdlms/obj.hpp"

Obj::Obj(AnimatedSprite animatedsprite,
         int layer,
         int z, std::u16string url) : AnimatedSprite(animatedsprite),
                                      _layer(layer), _z(z),
                                      _url(url)
{
}
